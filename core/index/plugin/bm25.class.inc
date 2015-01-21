<?php
require_once 'core/index/SyndIndexExtension.class.inc';

/**
 * Implementation of the BM25 weighting algorithm
 *
 * The constant default values are based on genetic algorithm 
 * evaluation using SyndGeneticEvaluator and the TREC9 collection;
 * the experiments suggest {K1=1, K2=0.05, B=0.5}.
 *
 * @package		synd.core.index
 * @see			SyndGeneticEvaluator
 * @link		http://www.xapian.org/docs/bm25.html
 * @link		http://trec.nist.gov
 */
class synd_plugin_bm25 extends SyndIndexExtension {
	/**
	 * Governs the importance of within document frequency.
	 *
	 * K1 controls the nonlinear term frequency effect. Must be >= 0
	 * where 0 means ignore Within Document Frequency (wdf). 
	 *
	 * @var		float
	 * @access	protected
	 */
	var $_K1 = null;

	/**
	 * Compensation factor for high wdf values
	 *
	 * K2 is a compensation factor for the high Within Document Frequency 
	 * (wdf) values in large documents. Must be >= 0.  0 means no 
	 * compensation. Default 
	 *
	 * @var		float
	 * @access	protected
	 */
	var $_K2 = null;

	/**
	 * Governs the importance of Within Query Frequency (wqf)
	 *
	 * Must be >= 0, where 0 means no compensation. This factor 
	 * defaults to 1.
	 *
	 * The SQL implementation doesn't allow this expression to be 
	 * computed, one term column in the result can map to several
	 * different terms all with different wqf's. Since the terms 
	 * in most queries only occur once (wqf of 1) it makes little
	 * difference.
	 *
	 * @var		float
	 * @access	protected
	 */
	var $_K3 = null;

	/**
	 * Relative importance of within document frequency and document length. 
	 *
	 * Controls the document length normalization. BM15 - BM25 Scaling Factor.
	 * Must be >= 0 and <= 1.
	 *
	 * @var		float
	 * @access	protected
	 */
	var $_B = null;

	function __construct($K1 = 1, $K2 = 1.25, $K3 = 1, $B = 0.5) {
		$this->_K1 = $K1;
		$this->_K2 = $K2;
		$this->_K3 = $K3;
		$this->_B = $B;
	}
	
	function initialize($index) {
		parent::initialize($index);
		$index->attachHook('query', array($this, '_callback_query'));
		$index->attachHook('query_expansion', array($this, '_callback_query_expansion'));
	}
	
	/**
	 * Builds a query.
	 * @param	SqlQueryBuilder	The query builder
	 * @param	IndexQuery			The query to run
	 * @param	SyndExpression		The expression to filter on
	 * @return	string
	 */
	function _callback_query(&$result, $builder, $query, $expression) {
		$builder->pushContext();
		
		$builder->order('WEIGHT', false);
		$docExpression = $this->documentWeight($builder, $query);

		if (0 == ($height = $expression->getHeight()))
			$builder->column($docExpression, 'WEIGHT', '*');
		else {
			$termExpressions = array();
			for ($i=0, $height=$expression->getHeight(); $i<$height; $i++)
				$termExpressions[] = $this->termDocumentWeight($builder, $query);
			
			// No need to do expensive GROUP BY if only one branch
			$visitor = new StatisticsExpressionVisitor($this->_index);
			$expression->acceptVisitor($visitor);
			
			if ($visitor->getBranches() <= 1)
				$builder->column("(".implode(') + (',$termExpressions).") + $docExpression", 'WEIGHT', '*');
			else {
				$builder->column("SUM(".implode(') + (',$termExpressions).") + $docExpression", 'WEIGHT', '*');
				$d = $builder->join('synd_search_document','d',array('docid'));
				$builder->groupBy("$d.ndl");
			}
		}

		$builder->popContext();
	}
	
	/**
	 * Builds an expansion query.
	 * @param	SqlQueryBuilder	The query builder
	 * @param	IndexQuery			The query to run
	 * @return	string
	 */
	function _callback_query_expansion(&$result, $builder, $query) {
		// Nomalized Document Length
		$ndl = $builder->join('synd_search_document','d',array('docid')).'.ndl';
		
		// Term's relevancy to document 
		$t = $builder->allocate('synd_search_termindex','t',array('docid','termid'));
		$rdt = $this->termRelevancy($ndl, "$t.wdf", "$t.wdw");

		// Term's base weight
		$td = $builder->allocate('synd_search_term','td',array('termid'));
		$builder->groupBy("$td.n");
		$wt = $this->termWeight(
			$builder->_index->getDocumentCount(), "$td.n",
			count($query->getRelevanceSet()), 'COUNT(*)');
			
		$builder->column("\n\tAVG($rdt) * \n\t$wt", 'WEIGHT', '*');
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
			return "LN((($r+0.5) * ($N-$n-$R+$r+0.5)) / (($n-$r+0.5) * ($R-$r+0.5)))";
		return "LN(($N-$n+0.5) / ($n+0.5))";
	}
	
	/**
	 * The relevancy of a term to a specific document
	 * @param	string	Normalized Document Length (ndl)
	 * @param	string	Within Document Frequency (wdf)
	 * @param	string	Within Document Weight (wdw)
	 * @return	string
	 */
	function termRelevancy($ndl, $wdf, $wdw) {
		$K   = "$this->_K1 * ($this->_B * $ndl + ".(1-$this->_B).")";
		$wdf = "$wdf * ($wdw/255)";
		return "((".($this->_K1+1)." * $wdf) / ($K + $wdf))";
	}

	/**
	 * Builds a Term-Document weight expression.
	 * @param	SqlQueryBuilder	The query builder
	 * @param	IndexQuery			The query to run
	 * @param	integer	The current termindex table
	 * @return	string
	 */
	function termDocumentWeight($builder, $query) {
		// Nomalized Document Length
		$ndl = $builder->join('synd_search_document','d',array('docid')).'.ndl';
		
		// Term's relevancy to document 
		$t = $builder->allocate('synd_search_termindex','t',array('docid','termid'));
		$rdt = $this->termRelevancy($ndl, "$t.wdf", "$t.wdw");

		// Term's base weight
		$wt = $this->termWeight(
			$builder->_index->getDocumentCount(), 
			$builder->allocate('synd_search_term','td',array('termid')).'.n');

		// Document-Term weight
		return "\n\t$rdt * \n\t$wt";
	}
	
	/**
	 * The weight of a document
	 * @param	SqlQueryBuilder	The query builder
	 * @param	IndexQuery			The query to run
	 * @return	string
	 */
	function documentWeight($builder, $query) {
		$ndl = $builder->join('synd_search_document','d',array('docid')).'.ndl';
		$nql = $query->getLength($builder->_index);

		return ($this->_K2 * $nql)." / (1 + $ndl)";
//		return (2 * $this->_K2 * $nql)." / (1 + $ndl)";
//		return ($this->_K2 * $nql)." * (1 - $ndl) / (1 + $ndl)";
	}
	
	function loadGene($gene) {
		list ($this->_K1, $this->_K2, $this->_K3, $this->_B, $ndl) = $gene;
	}

	/**
	 * @return	array
	 */
	function getDefaultGenes() {
		return array(
			array(1, 0.05, 1, 0.5, 0.25),	// TREC9 genetic first run
			array(1, 1.25, 1, 0.5, 0.15),	// TREC9 genetic later run
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
