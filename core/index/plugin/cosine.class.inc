<?php
require_once 'core/index/SyndIndexExtension.class.inc';

/**
 * Implementation of the Cosine weighting algorithm.
 *
 * The constants default values are based on genetic algorithm 
 * evaluation using SyndGeneticEvaluator and the TREC9 collection
 * and they should be quite optimal for most appliances.
 *
 * Local optima around K={0.215, 0.285, 0.31, 0.325}
 *
 * For more information see
 * <pre>
 *  Managing Gigabytes, Second Edition. p. 180-188
 *  Witten, I. H., Moffat, M., Bell, T. C. 1999. 
 * </pre>
 *  
 * @package		synd.core.index
 * @see			SyndGeneticEvaluator
 * @link		http://www.cs.mu.oz.au/mg/
 * @link		http://trec.nist.gov
 */
class synd_plugin_cosine extends SyndIndexExtension {
	/**
	 * K controls the balance of importance between initial and later
	 * appearances of a term within a document. Must be >= 0 and <= 1.
	 * @var		float
	 * @access	protected
	 */
	var $_K = 0.325;

	function __construct($K = 0.325) {
		$this->_K = $K;
	}

	function initialize($index) {
		parent::initialize($index);
		$index->attachHook('query', array($this, '_callback_query'));
		$index->attachHook('query_expansion', array($this, '_callback_query_expansion'));

		$index->attachHook('index_commit', array($this, '_callback_index_commit'));
		$index->attachHook('index_analyze', array($this, '_callback_index_analyze'));
		$index->attachHook('index_optimize', array($this, '_callback_index_optimize'));

		$index->attachHook('index_clear', array($this, '_callback_index_clear'));
		$index->attachHook('index_clear_section', array($this, '_callback_index_clear_section'));
		$index->attachHook('index_delete_document', array($this, '_callback_index_delete_document'));
		$index->attachHook('index_delete_batch', array($this, '_callback_index_delete_batch'));
	}

	/**
	 * Builds a query.
	 * @param	SqlQueryBuilder	The query builder
	 * @param	IndexQuery			The query to run
	 * @param	SyndExpression		The expression to filter on
	 * @return	string
	 */
	function _callback_query(&$result, $builder, $query, $expression) {
		$builder->order('WEIGHT', false);
		$docExpression = $this->documentWeight($builder, $query);

		if (0 == ($height = $expression->getHeight()))
			$builder->column($docExpression, 'WEIGHT', '*');
		else {
			$termExpressions = array();
			for ($i=0, $height=$expression->getHeight(); $i<$height; $i++)
				$termExpressions[] = $this->termDocumentWeight($builder, $query);
			$builder->column("SUM(".implode(') + (',$termExpressions).") * $docExpression", 'WEIGHT', '*');
		}
	}
	
	/**
	 * Builds an expansion weight expression.
	 * @param	SqlQueryBuilder	The query builder
	 * @param	IndexQuery			The query to run
	 * @param	string	Number of relevant documents indexed by term
	 * @return	string
	 */
	function _callback_query_expansion(&$result, $builder, $query) {
		$termWeight = $this->termWeight(
			$builder->_index->getDocumentCount(), 
			$builder->allocate('synd_search_term','td',array('termid')).'.n', 
			count($query->getRelevanceSet()), 'COUNT(*)');
		$builder->column("COUNT(*) * $termWeight", 'WEIGHT', '*');
		$builder->order('WEIGHT', false);
	}

	/**
	 * Term weight.
	 * @param	integer	Number of document in collection
	 * @param	integer	Number of documents indexed by term
	 * @param	integer	Number of relevant documents in collection
	 * @param	integer	Number of relevant documents indexed by term
	 * @return	string
	 */
	function termWeight($N, $n, $R = 0, $r = 0) {
		if (0 != $R || 0 != $r)
			return "LN(1 + (($r+0.5) / ($R-$r+0.5)) * (($n-$r+0.5) / ($N-$n-$R+$r+0.5)))";
		return "LN(1 + ($N+0.5) / ($n+0.5))";
	}

	/**
	 * Builds a Term-Document weight expression.
	 * @param	SqlQueryBuilder	The query builder
	 * @param	IndexQuery			The query to run
	 * @param	integer	The current termindex table
	 * @return	string
	 */
	function termDocumentWeight($builder, $query, $i) {
		// Nomalized Document Length
		$maxwdf = $builder->join('synd_search_cosine','c').'.maxwdf';
		$builder->groupBy($maxwdf);
		
		$t = $builder->allocate('synd_search_termindex','t',array('docid','termid'));
		
		// Total document count
		$N = $builder->_index->getDocumentCount();
		// Documents indexed by term
		$n = $builder->allocate('synd_search_term','td',array('termid')).'.n';
		
		// Term to document relevance
		$rdt = "($this->_K + (1-$this->_K) * ($t.wdf / $maxwdf))";
		// Term weight
		$wt = $this->termWeight($N, $n);
		
		// Document-Term weight
		return "\n\t$rdt * \n\t$wt * ($t.wdw/255)";
	}

	/**
	 * Builds a document weight expression.
	 * @param	SqlQueryBuilder	The query builder
	 * @param	IndexQuery			The query to run
	 * @return	string
	 */
	function documentWeight($builder, $query) {
		$builder->order('weight', false);
		$ndl = $builder->join('synd_search_document','d',array('docid')).'.ndl';
		$weight = $builder->join('synd_search_cosine','c').'.weight';
		return "($ndl / $weight)";
	}
	
	/**
	 * Update static document weights
	 * @access	protected
	 */
	function _callback_index_commit() {
		profiler_enter(__METHOD__);
		// TODO: Do this when adding documents
		$sql = "
			UPDATE synd_search_cosine
			SET maxwdf = (
				SELECT MAX(t.wdf) FROM synd_search_termindex t
				WHERE t.docid = synd_search_document.docid)
			WHERE maxwdf IS NULL";
		$this->_index->_db->query($sql);

		$N   = $this->_index->_db->getOne('SELECT COUNT(p.docid) FROM synd_search_document p');
		$wdt = $this->termDocumentWeight('t.wdf', $N, 'td.n', 't.wdw', 'synd_search_cosine.maxwdf');
		
		$sql = "
			UPDATE synd_search_cosine
			SET weight = (
				SELECT SQRT(SUM(POWER($wdt,2)))
				FROM synd_search_termindex t, synd_search_term td
				WHERE t.termid = td.termid AND t.docid = synd_search_document.docid)";
		if (1 != mt_rand(1,250))
			$sql ."\nWHERE weight IS NULL";
		$this->_index->_db->query($sql);
		
		profiler_leave(__METHOD__);
	}

	/**
	 * @access	private
	 */
	function _callback_index_analyze() {
		$this->_index->_db->analyze('synd_search_cosine');
	}

	/**
	 * @access	private
	 */
	function _callback_index_optimize() {
		$this->_index->_db->optimize('synd_search_cosine');
	}

	function _callback_index_clear() {
		$this->_index->_db->truncate('synd_search_cosine');
	}
	
	function _callback_index_clear_section(&$result, $section) {
		$sql = "
			DELETE FROM synd_search_cosine
			WHERE docid IN (
				SELECT d.docid FROM synd_search_document d
				WHERE d.section LIKE '$section.%')";
		$this->_index->_db->query($sql);
	}

	function _callback_index_delete_document(&$result, $pageId) {
		$sql = "
			DELETE FROM synd_search_cosine
			WHERE docid = ".$this->_index->documentId($pageId);
		$this->_index->_db->query($sql);
	}
	
	/**
	 * @access	private
	 */
	function _callback_index_delete_batch(&$result, $batch) {
		for ($i=0, $cnt=count($batch); $i<$cnt; $i+=1000) {
			$where = 'docid IN ('.implode(', ', array_slice($batch, $i, 1000)).')';
			$this->_index->_db->deleteQuick('synd_search_cosine', $where);
		}
	}

	function loadGene($gene) {
		list ($this->_K) = $gene;
	}

	function getDefaultGenes() {
		return array(array(0.325));
	}

	function getMinimumGene() {
		return array(0);
	}

	function getMaximumGene() {
		return array(1);
	}
}
