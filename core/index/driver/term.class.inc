<?php
require_once 'core/i18n.inc';
require_once 'core/index/DatabaseIndex.class.inc';
require_once 'core/index/SyndExpression.class.inc';
require_once 'core/index/SqlQueryBuilder.class.inc';

/**
 * Full featured and fast term index.
 *
 * Indexes every word in texts (using 32bit integers). Use the 
 * variety of extensions to customize behavior.
 *
 * @access		public
 * @package		synd.core.index
 * @link		http://www.xapian.org/docs/intro_ir.html
 */
class synd_index_term extends DatabaseIndex {
	/**
	 * Term postings buffer
	 * @access	protected
	 */
	var $_postings = array();

	/**
	 * Term buffer
	 * @access	private
	 */
	var $_terms = array();
	
	/**
	 * Max number of terms to index a document by
	 * @access	private
	 */
	var $_maxPostings = 8192;
	
	/**
	 * Deleted documents buffer
	 * @access	private
	 */
	var $_deleted = array();
	
	/**
	 * @access	private
	 */
	var $_lexer = null;

	function initialize() {
		parent::initialize();
		$this->attachHook('query_visit_term', array($this, '_callback_query_visit_term'), -100);
		$this->attachHook('query_visit_null', array($this, '_callback_query_visit_null'), -100);
		$this->attachHook('index_posting', array($this, '_callback_index_posting'), -100);
	}

	function loadExtension($extension) {
		$this->_lexer = null;
		parent::loadExtension($extension);
	}
	
	function getMatchSet(IndexQuery $query, $offset, $limit, $order = 'relevance') {
		profiler_enter(__METHOD__);

		if (null == ($expression = $query->createExpression($this)) || !$expression->isBounded()) {
			profiler_leave(__METHOD__);
			return array();
		}
		
		// Run expression hook
		$this->runHook('expression', array($expression));

		// Setup query builder
		$builder = new SqlQueryBuilder($this);
		$d = $builder->join('synd_search_document','d',array('docid'));
		
		// No need to do expensive GROUP BY if only one branch
		$visitor = new StatisticsExpressionVisitor($this);
		$expression->acceptVisitor($visitor);

		if ($visitor->getBranches() > 1 || count($query->getSecurityAttributes()) > 1)
			$builder->groupBy("$d.pageid");

		$builder->column("$d.docid", 'DOCID');
		$builder->column("$d.pageid", 'PAGEID');
		$builder->column("$d.section", 'SECTION');

		// Build filters from expression, section and attributes
		$builder->pushContext();
		$builder->filter($expression->getBalanced());
		$builder->where($this->sectionFilter($query->getSections(), $d));
		
		if (count($query->getSecurityAttributes())) {
			$a = $builder->join('synd_search_attribute','a',array('docid'));
			$builder->where($this->attributeFilter($query->getSecurityAttributes(), $a));
		}
		
		$builder->popContext();

		// Run query hook
		$this->runHook('query', array($builder, $query, $expression));
		
		// Execute query
		$sql = $builder->toString();
		$rows = $this->_db->getAll($sql, $offset, $limit, null, 
			SYND_SQL_CALC_FOUND_ROWS|SYND_SQL_HIGH_PRIORITY);
		
		if (SyndLib::isError($rows)) {
			trigger_error($rows->getMessage(), E_USER_WARNING);
			return array();
		}

		// Store found number of rows
		$query->setMatchCount($this->_db->getNamespace(), $this->_db->foundRows());

		profiler_leave(__METHOD__);
		return $rows;
	}
	
	function getMatchCount(IndexQuery $query) {
		assert('is_object($query)');
		if (null !== ($count = $query->getMatchCount($this->_db->getNamespace())))
			return $count;
		
		profiler_enter(__METHOD__);

		if (null == ($expression = $query->createExpression($this)) || !$expression->isBounded()) {
			profiler_leave(__METHOD__);
			return 0;
		}
		
		// Run expression hook
		$this->runHook('expression', array($expression));

		// Setup query builder
		$builder = new SqlQueryBuilder($this);
		$d = $builder->join('synd_search_document','d',array('docid'));

		// No need to do expensive DISTINCT if only one branch
		$visitor = new StatisticsExpressionVisitor($this);
		$expression->acceptVisitor($visitor);

		if ($visitor->getBranches() > 1 || count($query->getSecurityAttributes()) > 1)
			$builder->column("COUNT(DISTINCT $d.docid)");
		else
			$builder->column("COUNT(*)");
		
		// Build filters from expression, section and attributes
		$builder->pushContext();
		$builder->filter($expression->getBalanced());
		$builder->where($this->sectionFilter($query->getSections(), $d));
		
		if (count($query->getSecurityAttributes())) {
			$a = $builder->join('synd_search_attribute','a',array('docid'));
			$builder->where($this->attributeFilter($query->getSecurityAttributes(), $a));
		}
		
		$builder->popContext();

		// Execute query
		$sql = $builder->toString();
		//debug($sql);
		if (SyndLib::isError($count = $this->_db->getOne($sql))) {
			trigger_error($count->getMessage(), E_USER_WARNING);
			return 0;
		}

		profiler_leave(__METHOD__);
		return $count;
	}
	
	function getExpandSet(IndexQuery $query, $limit = 10) {
		if (!count($query->getRelevanceSet()))
			return array();

		profiler_enter(__METHOD__);
		
		// Setup query builder
		$builder = new SqlQueryBuilder($this);
		$builder->pushContext();
		$t = $builder->allocate('synd_search_termindex','t',array('docid','termid'));
		$td = $builder->allocate('synd_search_term','td',array('termid'));
		$builder->popContext();

		// Build filters from relevance set
		$builder->column("$td.TERM");
		$builder->where("$t.docid IN (".implode(', ', $this->_db->quote($this->documentId($query->getRelevanceSet()))).")");
		$builder->where("$td.n > 1");
		$builder->groupBy("$t.termid");

		// Run expansion query hook
		$this->runHook('query_expansion', array($builder, $query));

		// Execute query
		$sql = $builder->toString();
		if (SyndLib::isError($terms = $this->_db->getCol($sql, 0, 0, $limit))) {
			trigger_error($terms->getMessage(), E_USER_WARNING);
			return array();
		}
			
		profiler_leave(__METHOD__);
		return $terms;
	}

	function addDocument($pageid, $document, $section = null, $attributes = null) {
		// Build NULL posting to facilitate query-branch balancing
		$this->_postings[] = array(
			'docid' => $this->documentId($pageid), 'termid' => 0,
			'field' => 0, 'context' => 0, 'wdf' => 0, 'wdw' => 0);
			
		parent::addDocument($pageid, $document, $section, $attributes);
		if (count($this->_postings) >= 7500 && SyndLib::isError($result = $this->flush()))
			trigger_error($result->getMessage(), E_USER_ERROR);
	}
	
	/**
	 * Adds the documents terms to index.
	 *
	 * The 'index_posting' hook is run directly instead of via runHook()
	 * to get a 20% speedup, the callback method must therefore be
	 * on the form array($extension, '_callback_index_posting').
	 */
	function visitDocumentFragment($document) {
		parent::visitDocumentFragment($document);
		profiler_enter(__METHOD__);

		$docid = $this->_context['docid'];
		if (($count = $this->_maxPostings - $this->_documents[$docid]['length']) <= 0)
			return;

		profiler_enter('preg_split');
		$terms = preg_split("/[^{$SYND_WORDCHARS}]+/S", 
			synd_strtolower($document->toString()), $count, PREG_SPLIT_NO_EMPTY);
		profiler_leave('preg_split');
		
		// Increase document length
		if (count($terms) < $count) {
			$this->_documents[$docid]['length'] += count($terms);
			if (null == end($terms))
				array_pop($terms);
		}
		else {
			$this->_documents[$docid]['length'] = $this->_maxPostings;
			array_pop($terms);
		}
		
		$hooks = $this->_hooks['index_posting'];
		$wdw = $this->getWithinDocumentWeight($document);
		
		profiler_enter('add postings');

		// Add term postings
		foreach ($terms as $term) {
			if (strlen($term) > $this->_maxTermLength)
				$term = substr($term, 0, $this->_maxTermLength);
			
			$original = $term;
			$context = 0;
			$field = 0;

			// Run hook directly to get higher performance
			foreach ($hooks as $hook) {
				if ($hook[0]->_callback_index_posting(null, $docid, $term, $field, $context, $wdw, null, $original))
					break;
			}
		}

		profiler_leave('add postings');
		profiler_leave(__METHOD__);
	}

	function _callback_index_posting($result, $docid, &$term, &$field, &$context, $wdw, $nonChunked = null, $original = null) {
		if (null == $term)
			return;
		
		$key = $docid.'.'.$term;
		$id = $this->termId($term);
		
		// Add the posting to the synd_search_termindex table
		if (!isset($this->_postings[$key])) {
			$this->_postings[$key] = array(
				'docid' => $docid,
				'termid' => $id,
				'field' => $field,
				'context' => $context,
				'wdf' => 1,
				'wdw' => $wdw);
			
			// Increase term frequency
			if (isset($this->_terms[$id]))
				$this->_terms[$id]['N']++;
			else {
				// Add the term to the synd_search_term table
				$this->_terms[$id] = array(
					'TERMID' => $id, 
					'TERM' => $term, 
					'ORIGINAL' => $this->_db->quote(null != $original ? $original : $term),
					'FUZZY' => 'NULL',
					'N' => 1);

				$this->runHook('index_term', array($this->_terms[$id]));
				$this->_terms[$id]['TERM'] = "'".$this->_terms[$id]['TERM']."'";
			}
		}
		else {
			// Merge field hash
			$this->_postings[$key]['field'] |= $field;
			
			// Merge context hash
			$this->_postings[$key]['context'] |= $context;

			// Increase within document frequency
			$this->_postings[$key]['wdf']++;

			// Max within document weight
			if ($this->_postings[$key]['wdw'] < $wdw)
				$this->_postings[$key]['wdw'] = $wdw;
		}
	}
	
	function getWithinDocumentWeight($document) {
		return ceil(pow($document->getWeight(), 2) * 255);
	}
	
	function delDocument($pageid) {
		$this->_deleted[] = $this->documentId($pageid);
		parent::delDocument($pageid);
	}

	function delDocumentBatch($batch) {
		$this->_deleted = array_merge($this->_deleted, $batch);
		parent::delDocumentBatch($batch);
	}

	function clear() {
		$this->_db->truncate('synd_search_termindex');
		$this->_db->truncate('synd_search_term');
		
		// Insert the NULL term to be used when balancing expressions
		$this->_db->query("INSERT INTO synd_search_term (termid, n) VALUES (0, 0)");
		
		// Release memory properly
		unset($this->_postings, $this->_terms);
		$this->_postings = array();
		$this->_terms = array();

		parent::clear();
	}

	function clearSection($section) {
		profiler_enter(__METHOD__);
		
		$where = "docid IN (
			SELECT d.docid FROM synd_search_document d
			WHERE d.section LIKE '$section.%')";
		$this->_db->deleteQuick('synd_search_termindex', $where);
		$this->_termStatistics();
		
		profiler_leave(__METHOD__);
		parent::clearSection($section);
	}

	/**
	 * Flush term buffers to database
	 */
	function _flush() {
		if (!$this->inTransaction()) {
			// Merge term frequencies
			for ($i=0, $cnt=count($ids=array_keys($this->_terms)); $i<$cnt; $i+=1000) {
				$sql = "
					SELECT td.termid, td.n FROM synd_search_term td
					WHERE td.termid IN (".implode(', ', $this->_db->quote(array_slice($ids, $i, 1000))).")";
				
				$freqs = $this->_db->getAssoc($sql);
				foreach ($freqs as $id => $n)
					$this->_terms[$id]['N']	+= $n;
			}

			// Compensate for deleted documents
			for ($i=0, $cnt=count($this->_deleted); $i<$cnt; $i+=1000) {
				$sql = "
					SELECT td.TERMID, td.TERM, td.ORIGINAL, td.FUZZY, td.N, COUNT(*) NDELETED
					FROM synd_search_term td, synd_search_termindex t
					WHERE 
						td.termid = t.termid AND 
						t.docid IN (".implode(', ', $this->_db->quote(array_slice($this->_deleted, $i, 1000))).")
					GROUP BY td.termid, td.term, td.original, td.fuzzy, td.n";
				
				$freqs = $this->_db->getAll($sql);
				$delete = array();

				foreach ($freqs as $term) {
					if (isset($this->_terms[$term['TERMID']]))
						$this->_terms[$term['TERMID']]['N'] -= $term['NDELETED'];
					else if ($term['N'] <= $term['NDELETED']) 
						$delete[] = $term['TERMID'];
					else {
						$term['N'] -= $term['NDELETED']; 
						unset($term['NDELETED']);
						$this->_terms[$term['TERMID']] = $this->_db->quote($term);
					}
				}

				// Delete now obsolete terms
				for ($j=0, $jcnt=count($delete); $j<$jcnt; $j+=1000)
					$this->_db->deleteQuick('synd_search_term', 'termid IN ('.implode(', ', $this->_db->quote(array_slice($delete, $j, 1000))).')');
			}
		}
		
		// Flush parent
		if (SyndLib::isError($result = parent::_flush()))
			return $result;
		
		// Flush term data
		if (SyndLib::isError($result = $this->_db->load('synd_search_term', $this->_terms)))
			return $result;
		
		// Flush term index
		if (SyndLib::isError($result = $this->_db->load('synd_search_termindex', $this->_postings)))
			return $result;

		// Release memory properly
		unset($this->_postings, $this->_terms, $this->_deleted);
		$this->_postings = array();
		$this->_terms = array();
		$this->_deleted = array();
	}

	/**
	 * @access	private
	 */
	function _termStatistics() {
		profiler_enter(__METHOD__);
		
		$sql = "
			UPDATE synd_search_term 
			SET n = (
				SELECT COUNT(t.docid) 
				FROM synd_search_termindex t 
				WHERE t.termid = synd_search_term.termid) 
			WHERE termid != 0";
		
		$this->_db->query($sql);
		$this->_db->deleteQuick('synd_search_term', 'n = 0 AND termid != 0');

		profiler_leave(__METHOD__);
	}

	/**
	 * Update term posting count (n)
	 */
	function commit() {
		parent::commit();
		$this->_termStatistics();
	}
	
	function analyze() {
		parent::analyze();
		$this->_termStatistics();
		$this->_db->analyze('synd_search_term');
		$this->_db->analyze('synd_search_termindex');
	}
	
	function optimize() {
		parent::optimize();
		$this->_termStatistics();
		$this->_db->optimize('synd_search_term');
		$this->_db->optimize('synd_search_termindex');
	}

	/**
	 * Returns the number of indexed terms
	 * @return	integer
	 */
	function getTermCount() {
		$sql = 'SELECT COUNT(*) FROM synd_search_term';
		if (SyndLib::isError($terms = $this->_db->getOne($sql)))
			return trigger_error($sql, E_USER_WARNING);
		return $terms > 1 ? $terms : 0;
	}

	/**
	 * Returns the number of term postings, {term,docid} pairs
	 * @param	string	Term to count postings for
	 * @return	integer
	 */
	function getPostingCount($term = null) {
		$sql = 'SELECT COUNT(*) FROM synd_search_termindex t';
		if (null != $term)
			$sql .= " WHERE t.termid = ".$this->termId($term);
		if (SyndLib::isError($postings = $this->_db->getOne($sql)))
			return trigger_error($sql, E_USER_WARNING);
		return $postings > 1 ? $postings : 0;
	}
	
	function termId($term) {
		return 0 != ($id= crc32($term)) ? $id : 1;
	}

	/**
	 * Default extension hook
	 * @access	protected
	 */
	function _callback_query_visit_term(&$result, $builder, $expression) {
		$t = $builder->allocate('synd_search_termindex','t',array('docid','termid'));
		$result = "$t.termid = ".$this->termId($expression->toString());
	}

	function _callback_query_visit_null(&$result, $builder, $expression) {
		$t = $builder->allocate('synd_search_termindex','t',array('docid','termid'));
		$result = "$t.termid = 0";
	}
}

/**
 * Check for the existance of OR branches
 * @package	synd.core.index
 */
class StatisticsExpressionVisitor implements IndexExpressionVisitor {
	var $_index = null;
	var $_branches = 1;
	var $_terms = array();
	
	function StatisticsExpressionVisitor($index) {
		$this->_index = $index;
	}
	
	function visitOperand($operand) {
		if (($operand instanceof SyndTermQuery)) {
			$this->_branches += $this->_index->runHook('query_branched_term', array($this, $operand));
			$this->_terms[] = $operand;
		}
	}

	function visitUnaryOperator($operator, $operand) {
		$this->_branches += $this->_index->runHook('query_branched_operator', array($this, $operator));
		$operand->acceptVisitor($this);
	}

	function visitBinaryOperator($operator, $lhs, $rhs) {
		if (($operator instanceof SyndBooleanOR))
			$this->_branches++;
		else if (null !== ($branches = $this->_index->runHook('query_branched_operator', array($this, $operator))))
			$this->_branches += $branches;
		else {
			$lhs->acceptVisitor($this);
			$rhs->acceptVisitor($this);
		}
	}

	function getBranches() {
		return $this->_branches;
	}
	
	function getTerms() {
		return $this->_terms;
	}
}
