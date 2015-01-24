<?php
require_once 'core/index/AbstractIndex.class.inc';
require_once 'core/index/SyndExpression.class.inc';
require_once 'core/index/IndexBuilder.class.inc';

define('XAPIAN_VALUENO_BIAS', 0);
define('XAPIAN_VALUENO_PAGEID', 1);
define('XAPIAN_VALUENO_SECTION', 2);
define('XAPIAN_VALUENO_ATTRIBUTES', 3);

/**
 * Xapian wrapper.
 *
 * Xapian is an Open Source Probabilistic Information Retrieval library, 
 * released under the GPL. It's written in C++, and bindings are under 
 * development to allow use from other languages.
 *
 * It requires the Xapian php module to be loaded. To do this simply
 * download the Xapian core and SWIG bindings and compile. 
 *
 * Ensure that the path to store the index is writable by the 
 * webserver. If no index exists at the specifed path one will be 
 * created automatically when first used.
 *
 * @link		http://www.xapian.org/
 */
class synd_index_xapian extends AbstractIndex {
	/**
	 * Path to Xapian database (should be writable to the webserver)
	 * @var		string
	 * @access	protected
	 */
	var $_dsn = null;

	/**
	 * Remove lockfile if exists
	 * @var		bool
	 * @access	protected
	 */
	var $_removelock = null;

	/**
	 * Database handle
	 * @var		XapianDatabase
	 * @access	protected
	 */
	var $_db = null;
	
	/**
	 * Writable database handle
	 * @var		XapianWritableDatabase
	 * @access	protected
	 */
	var $_writable = null;
	
	/**
	 * Enquire object in use
	 * @var		XapianEnquire
	 * @access	protected
	 */
	var $_enquire = null;
	
	/**
	 * Loaded weighting scheme
	 * @var		XapianBM25WeightingScheme
	 * @access	protected
	 */
	var $_weight = null;
	
	/**
	 * Max number of terms to index a document by
	 * @access	protected
	 */
	var $_maxPostings = 8192;
	
	/**
	 * Terms will be chopped at this length
	 * @access	protected
	 */
	var $_maxTermLength = 32;
	
	function __construct($dsn, $removelock = true) {
		parent::__construct();

		$this->_dsn = $dsn;
		$this->_removelock = $removelock;

		$this->attachHook('builder', array($this, '_callback_builder'), self::PRIO_BUILDER_SINK);
	}

	function getDatabase() {
		if (null !== $this->_writable)
			return $this->_writable;
		if (null === $this->_db)
			$this->_db = new XapianDatabase($this->_dsn);
		return $this->_db;
	}
	
	function getWritableDatabase() {
		if (null === $this->_writable)
			$this->_writable = new XapianWritableDatabase($this->_dsn, $this->_removelock);
		return $this->_writable;
	}
	
	function getWeightingScheme() {
		if (null === $this->_weight)
			$this->getEnquire();
		return $this->_weight;
	}
	
	function getEnquire() {
		if (null === $this->_enquire) {
			$this->_enquire = new XapianEnquire(new_Enquire($this->getDatabase()->getHandle()));
			$this->_weight = new XapianBM25WeightingScheme($this->_enquire);
			$this->_enquire->set_weighting_scheme($this->_weight);
		}
		return $this->_enquire;
	}

	function lexerExpression() {
		return array_merge((array)$this->runHook('query_lexer_expression'), array(
				'[()]',
				'[&|]{1,2}',
				'(?<!\w)[+\-!]',
				'(?<![a-zедц\d])[a-zедц\d]{1,'.$this->_maxTermLength.'}'));
	}
	
	function createExpression($token) {
		switch ($token) {
			case '+':
				$expression = new SyndBooleanInclude();
				break;

			case '-':
			case '!':
				$expression = new SyndBooleanExclude();
				break;

			case '&':
			case '&&':
				$expression = new SyndBooleanAND();
				break;

			case '|':
			case '||':
				$expression = new SyndBooleanOR();
				break;

			default: 
				$expression = new SyndTermQuery(strtolower($token));
		}
		return $expression;
	}

	private function _parse($query, $force = false) {
		if (null == ($expression = $query->createExpression($this)) && !$force)
			return null;
		
		if (null != $expression) {
			if (!$expression->isBounded())
				return null;
			
			$builder = new XapianQueryBuilder($this);
			$expression->acceptVisitor($builder);
			if (null == ($filters = $builder->getExpression()) && !$force)
				return null;
		}

		// Filter documents on section
		if (count($sections = $query->getSections())) {
			$filter = new_Query(md5('__section__:'.array_pop($sections)));
			while (null !== ($section = array_pop($sections)))
				$filter = new_Query(Query_OP_OR, new_Query(md5('__section__:'.$section)), $filter);
			$filters = isset($filters) ? new_Query(Query_OP_AND, $filters, $filter) : $filter;
		}
		
		// Filter documents on security context
		if (count($attributes = $query->getSecurityAttributes())) {
			$filter = new_Query(md5('__attribute__:'.array_pop($attributes)));
			while (null !== ($attribute = array_pop($attributes)))
				$filter = new_Query(Query_OP_OR, new_Query(md5('__attribute__:'.$attribute)), $filter);
			$filters = isset($filters) ? new_Query(Query_OP_AND, $filters, $filter) : $filter;
		}
		
		return $filters;
	}
	
	private function _section($patterns, $section) {
		if (empty($section))
			return true;
		if (empty($patterns))
			return false;
		foreach ($patterns as $pattern) {
			if (0 === strpos($section, $pattern))
				return true;
		}
		return false;
	}

	private function _security($patterns, $attributes) {
		if (empty($attributes))
			return true;
		if (empty($patterns))
			return false;
		foreach ($patterns as $pattern) {
			foreach ($attributes as $attribute) {
				if (0 === strpos($attribute, $pattern))
					return true;
			}
		}
		return false;
	}

	function getMatchSet(IndexQuery $query, $offset, $limit, $order = 'relevance') {
		if (null == ($q = $this->_parse($query)))
			return array();

		profiler_enter(__METHOD__);
		$enquire = $this->getEnquire();
		$enquire->set_query($q);

		if ($order == 'date') {
			$enquire->set_weighting_scheme(new XapianBoolWeightingScheme());
                        $enquire->set_docid_order(Enquire_DESCENDING);
                }

		if (count($docids = $query->getRelevanceSet())) {
			$rset = new_RSet();
			foreach ($docids as $docid)
				RSet_add_document($rset, $docid);
			$mset = $enquire->get_mset($offset, $limit, $rset);
		}
		else {
			$mset = $enquire->get_mset($offset, $limit);
		}

		$matches = array();
		$sections = $query->getSections();
		$attributes = $query->getSecurityAttributes();

		$iterator = $mset->begin();
		$end = $mset->end();

		while (!$iterator->equals($end) && null != ($document = $iterator->get_document()) && (null == $limit || count($matches) < $limit)) {
			$section = $document->get_value(XAPIAN_VALUENO_SECTION);

			if ($this->_section($sections, $section) && $this->_security($attributes, 
				array_filter(explode(':',$document->get_value(XAPIAN_VALUENO_ATTRIBUTES)),'strlen'))) {
				$matches[] = array(
					'DOCID'		=> $iterator->get_docid(),
					'PAGEID'	=> $document->get_value(XAPIAN_VALUENO_PAGEID),
					'SECTION'	=> $section, 
					'WEIGHT'	=> $iterator->get_weight());
			}

			$iterator->next();
		}
		
		$query->_xapian_mset_count = $mset->get_matches_estimated();

		profiler_leave(__METHOD__);
		return $matches;
	}
	
	function getMatchCount(IndexQuery $query) {
		if (!isset($query->_xapian_mset_count))
			$this->getMatchSet($query, 0, 1);
		return isset($query->_xapian_mset_count) ? $query->_xapian_mset_count : 0;
	}

	function getExpandSet(IndexQuery $query, $limit = 10) {
		$rset = new_RSet();
		foreach ($query->getRelevanceSet() as $docid)
			RSet_add_document($rset, $docid);

		if (null == ($q = $this->_parse($query, true)))
			return array();
			
		$enquire = $this->getEnquire();
		$enquire->set_query($q);
		
		$eset = $enquire->get_eset($limit, $rset);
		if ($eset->is_empty())
			return array();
		
		$terms = array();
		$iterator = $eset->begin();
		$end = $eset->end();
		
		while (!$iterator->equals($end) && null != ($term = $iterator->get_termname())) {
			$terms[] = $term;
			$iterator->next();
		}
		
		return $terms;
	}
	
	function documentId($pageid) {
		$q = null;
		foreach ((array)$pageid as $id)
			$q = null != $q ? new_Query(Query_OP_OR, new_Query(md5('__pageid__:'.$id)), $q) : new_Query(md5('__pageid__:'.$id));
		
		$enquire = $this->getEnquire();
		$enquire->set_query($q);
		
		$docids = array();
		$mset = $enquire->get_mset(0, count((array)$pageid));

		$iterator = $mset->begin();
		$end = $mset->end();

		while (!$iterator->equals($end)) {
			$docids[] = $iterator->get_docid();
			$iterator->next();
		}
		
		return is_array($pageid) ? $docids : reset($docids);
	}

	function begin() {
		parent::begin();
		if (null != $this->_writable)
			$this->_writable->begin_transaction();
	}

	function commit() {
		parent::commit();
		if (null != $this->_writable)
			$this->_writable->commit_transaction();
	}
	
	function _callback_builder(&$result) {
		$result = new XapianIndexBuilder($this->getWritableDatabase(), $this->_maxTermLength, $this->_maxPostings);
	}		

	function delDocument($pageid) {
		$db = $this->getWritableDatabase();
		$db->delete_document(md5("__pageid__:$pageid"));
	}
	
	function delDocumentBatch($pageids) {
		$db = $this->getWritableDatabase();
		foreach ($pageids as $pageid)
			$db->delete_document(md5("__pageid__:$pageid"));
	}
	
	function getDocumentCount($section = null) {
		if (null == $section) {
			$db = $this->getDatabase();
			return $db->get_doccount();
		}
		
		$query = new_Query(md5("__section__:$section"));
		$enquire = $this->getEnquire();
		$enquire->set_query($query);
		
		$mset = $enquire->get_mset(0, 2500);
		$count = $mset->get_matches_estimated();

		return $count;
	}
	
	function getDocumentDiskUsage() {
		return SyndLib::file_total_size($this->_dsn);
	}

	function clear() {
		if (is_dir($this->_dsn) && null != ($list = SyndLib::scanDirectory($this->_dsn))) {
			foreach ($list as $file)
				SyndLib::unlink($file);
		}
		
		$this->_db = null;
		$this->_writable = null;
		
		parent::clear();
	}

	function clearSection($section) {
		$db = $this->getWritableDatabase();
		$db->delete_document(md5("__section__:$section"));
		
		parent::clearSection($section);
	}

	function termProcess($term) {
		$this->runHook('query_process_term', array(&$term));
		return $term;
	}
}

/**
 * Postfix visitor that builds a Xapian::Query
 * @package		synd.core.index
 */
class XapianQueryBuilder implements IndexExpressionVisitor {
	protected $_index = null;
	protected $_stack = array();
	
	function __construct($index) {
		$this->_index = $index;
	}
	
	function getExpression() {
		return end($this->_stack);
	}

	function visitOperand($operand) {
		if (($operand instanceof SyndTermQuery))
			$this->_index->runHook('query_visit_term', array($this, $operand));
		$this->_stack[] = new_Query($operand->toString());
	}
	
	function visitUnaryOperator($operator, $rhs) {
		$rhs->acceptVisitor($this);
	}
	
	function visitBinaryOperator($operator, $lhs, $rhs) {
		$rhs->acceptVisitor($this);
		$lhs->acceptVisitor($this);
		
		if (($operator instanceof SyndBooleanExclude)) 
			$operator = Query_OP_AND_NOT;
		else if (($operator instanceof SyndBooleanInclude)) 
			$operator = Query_OP_AND_MAYBE;
		else if (($operator instanceof SyndBooleanAND))
			$operator = Query_OP_AND;
		else if (($operator instanceof SyndBooleanOR))
			$operator = Query_OP_OR;
		else {
			$lhs->acceptVisitor($this);
			$rhs->acceptVisitor($this);
			return;
		}

		array_push($this->_stack, new_Query($operator, array_pop($this->_stack), array_pop($this->_stack)));
	}
}

/**
 * @access	private
 */
class XapianDatabase {
	protected $_isWritable = false;
	protected $_handle = null;
	protected $_dsn = null;

	function __construct($dsn) {
		$this->_dsn = $dsn;
		$this->_openDatabase($dsn);
	}

	function __destruct() {
		$this->_handle = null;
	}
	
	function getHandle() {
		return $this->_handle;
	}
	
	function _openDatabase($dsn) {
		// Try to automatically create the database if it doesn't exist
		if (!file_exists(rtrim($dsn,'/').'/flintlock')) {
			try {
				$handle = new_WritableDatabase($dsn, DB_CREATE_OR_OPEN);
				if ($handle != null)
					Database_close($handle);
			}
			catch (DatabaseLockError $e) {}
		}
		
		$this->_handle = new_Database($dsn);
	}
	
	function get_doccount() {
		return Database_get_doccount($this->_handle);
	}
}

/**
 * @access	private
 */
class XapianWritableDatabase extends XapianDatabase {
	protected $_removelock = null;
	
	function __construct($dsn, $removelock = true) {
		$this->_removelock = $removelock;
		parent::__construct($dsn);
	}

	function _openDatabase($dsn) {
		if ($this->_removelock && file_exists($lock = rtrim($dsn, DIRECTORY_SEPARATOR).'/db_lock'))
			unlink($lock);
		$this->_handle = new_WritableDatabase($dsn, DB_CREATE_OR_OPEN);
		$this->_isWritable = true;
	}
	
	function add_document($document) {
		if (null != $pageid)
			return WritableDatabase_add_document($this->_handle, $document);
		return false;
	}
	
	function replace_document($pageid, $document) {
		if (null != $pageid)
			return WritableDatabase_replace_document($this->_handle, $pageid, $document);
		return false;
	}
	
	function delete_document($pageid) {
		if (null != $pageid)
			WritableDatabase_delete_document($this->_handle, $pageid);
		return false;
	}

	function flush() {
		if (null !== $this->_handle)
			WritableDatabase_flush($this->_handle);
	}

	function begin_transaction() {
//		WritableDatabase_begin_transaction($this->_handle);
	}

	function commit_transaction() {
//		WritableDatabase_commit_transaction($this->_handle);
	}

	function cancel_transaction() {
//		WritableDatabase_cancel_transaction($this->_handle);
	}
}

/**
 * @access	private
 */
class XapianDocument {
	protected $_handle = null;
	
	function __construct($handle) {
		$this->_handle = $handle;
	}
	
	function __destruct() {
		$this->_handle = null;
	}
	
	function get_data() {
		return Document_get_data($this->_handle);
	}
	
	function get_value($valueno) {
		return Document_get_value($this->_handle, $valueno);
	}
}

/**
 * @access	private
 */
class XapianEnquire {
	protected $_handle = null;
	
	function __construct($handle) {
		$this->_handle = $handle;
	}
	
	function __destruct() {
		$this->_handle = null;
	}
	
	function set_query($query) {
		Enquire_set_query($this->_handle, $query);
	}
	
	function get_mset($offset, $limit, $rset = null) {
		return null != $rset ?
			new XapianMSet(Enquire_get_mset($this->_handle, (int)$offset, (int)$limit, 250, $rset)) : 
			new XapianMSet(Enquire_get_mset($this->_handle, (int)$offset, (int)$limit));
	}

	function get_eset($limit, $rset) {
		return new XapianESet(Enquire_get_eset($this->_handle, $limit, $rset));
	}
	
	function set_weighting_scheme($scheme) {
		Enquire_set_weighting_scheme($this->_handle, $scheme->getHandle());
	}

	function set_docid_order($order) {
		Enquire_set_docid_order($this->_handle, $order);
	}
	
	function set_bias($maxWeight, $halflife) {
		Enquire_set_bias($this->_handle, $maxWeight, $halflife);
	}
}

/**
 * @access	private
 */
class XapianMSet {
	protected $_handle = null;

	function __construct($handle) {
		$this->_handle = $handle;
	}
	
	function __destruct() {
		$this->_handle = null;
	}
	
	function fetch() {
		MSet_fetch($this->_handle);
	}
	
	function begin() {
		return new XapianMSetIterator(MSet_begin($this->_handle));
	}

	function end() {
		return new XapianMSetIterator(MSet_end($this->_handle));
	}
	
	function size() {
		return MSet_size($this->_handle);
	}
	
	function get_matches_estimated() {
		return MSet_get_matches_estimated($this->_handle);
	}
}

/**
 * @access	private
 */
class XapianMSetIterator {
	protected $_handle = null;
	
	function __construct($handle) {
		$this->_handle = $handle;
	}

	function __destruct() {
		$this->_handle = null;
	}
	
	function get_document() {
		return new XapianDocument(MSetIterator_get_document($this->_handle));
	}
	
	function get_docid() {
		return MSetIterator_get_docid($this->_handle);
	}
	
	function get_weight() {
		return MSetIterator_get_weight($this->_handle);
	}
	
	function next() {
		MSetIterator_next($this->_handle);
	}
	
	function equals(XapianMSetIterator $iterator) {
		return MSetIterator_equals($this->_handle, $iterator->_handle);
	}
}

/**
 * @access	private
 */
class XapianESet {
	protected $_handle = null;
	
	function __construct($handle) {
		$this->_handle = $handle;
	}
	
	function __destruct() {
		$this->_handle = null;
	}
	
	function begin() {
		return new XapianESetIterator(ESet_begin($this->_handle));
	}

	function end() {
		return new XapianESetIterator(ESet_end($this->_handle));
	}
	
	function is_empty() {
		return ESet_is_empty($this->_handle);
	}
}

/**
 * @access	private
 */
class XapianESetIterator {
	protected $_handle = null;
	
	function __construct($handle) {
		$this->_handle = $handle;
	}
	
	function __destruct() {
		$this->_handle = null;
	}
	
	function get_termname() {
		return ESetIterator_get_termname($this->_handle);
	}
	
	function next() {
		ESetIterator_next($this->_handle);
	}
	
	function equals(XapianESetIterator $iterator) {
		return ESetIterator_equals($this->_handle, $iterator->_handle);
	}
}

class XapianBoolWeightingScheme {
        protected $_handle = null;

        function __construct() {
                $this->_handle = new_BoolWeight();
        }

        function __destruct() {
                $this->_handle = null;
        }

        function getHandle() {
                return $this->_handle;
        }
}

/**
 * @access	private
 */
class XapianBM25WeightingScheme {
	protected $_handle = null;
	protected $_enquire = null;
	
	function __construct($enquire) {
		$this->_handle = new_BM25Weight();
		$this->_enquire = $enquire;
	}

	function __destruct() {
		$this->_handle = null;
	}
	
	function getHandle() {
		return $this->_handle;
	}
	
	function loadGene($gene) {
		list ($K1, $K2, $K3, $B, $ndl) = $gene;
		$this->_handle = new_BM25Weight($K1, $K2, $K3, $B, $ndl);
		$this->_enquire->set_weighting_scheme($this);
	}

	/**
	 * @return	array
	 */
	function getDefaultGenes() {
		return array(
			array(1, 0.05, 1, 0.5, 0.25),	// TREC9 genetic first run

			array(1, 1.25, 1, 0.5, 0.15),
			array(1.133, 0.66, 0.66, 0.66, 0.66),
			array(1.133, 1.133, 0.66, 0.66, 0.66),
			array(1.2, 7, 0.75, 1.2, 0),
			array(1.2, 1.2, 0.8, 1.2, 0.13),

			array(1, 0, 1, 0.5, 0.5), 		// Xapian default
			array(1.2, 0, 7, 0.75, 0.5), 	// Robertson
			);
	}

	function getMinimumGene() {
		return array(0,0,0,0,0);
	}

	function getMaximumGene() {
		return array(10,10,10,1,10);
	}
}

class XapianIndexBuilder implements IndexBuilder {
	protected $_database = null;
	protected $_postings = 0;
	protected $_pageid = null;
	protected $_document = null;
	protected $_maxlen = null;
	protected $_maxpostings = null;
	
	function __construct(XapianWritableDatabase $database, $maxlen, $maxpostings) {
		$this->_database = $database;
		$this->_maxlen = $maxlen;
		$this->_maxpostings = $maxpostings;
	}
	
	function open($pageid, $section = null, $attributes = array(), $modified = 0) {
		$this->_pageid = $pageid;
		$this->_document = new_Document();
		$this->_postings = 0;
		
		Document_add_value($this->_document, XAPIAN_VALUENO_BIAS, $modified ? $modified : time());
		Document_add_value($this->_document, XAPIAN_VALUENO_PAGEID, $pageid);
		Document_add_posting($this->_document, md5("__pageid__:$pageid"), $this->_postings++);
		
		// Add document section to values and index postings
		if (null != $section) {
			Document_add_value($this->_document, XAPIAN_VALUENO_SECTION, $section);
			for ($i=strpos($section,'.'); $i; $i=strpos($section,'.',$i+1))
				Document_add_posting($this->_document, md5("__section__:".substr($section,0,$i)), $this->_postings++);
			Document_add_posting($this->_document, md5("__section__:$section"), $this->_postings++);
			$this->_postings++;
		}
		
		// Add security context to values and index postings
		if (null != $attributes) {
			Document_add_value($this->_document, XAPIAN_VALUENO_ATTRIBUTES, implode(':', $attributes));
			foreach ($attributes as $attribute) {
				for ($i=strpos($attribute,'.'); $i; $i=strpos($attribute,'.',$i+1))
					Document_add_posting($this->_document, md5("__attribute__:".substr($attribute,0,$i)), $this->_postings++);
				Document_add_posting($this->_document, md5("__attribute__:$attribute"), $this->_postings++);
				$this->_postings++;
			}
		}
	}

	function close() {
		$this->_database->replace_document(md5('__pageid__:'.$this->_pageid), $this->_document);
		$this->_document = null;
	}
	
	function flush() {
		$this->_database->flush();
	}
	
	function visitFile($file, $type = null, $buffer = null) {}
	
	function visitFragment($fragment, $field = null, $weight = 1) {
		$this->_postings++;
	}

	function visitTerm($term, $field = null) {
		if ($this->_postings < $this->_maxpostings) {
			if (strlen($term) > $this->_maxlen)
				$term = substr($term, 0, $this->_maxlen);
			Document_add_posting($this->_document, $term, $this->_postings++);
		}
	}
}
