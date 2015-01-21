<?php
require_once 'core/index/SyndIndexExtension.class.inc';

/**
 * Takes Last-Modified into account when weighting documents
 *
 * @package		synd.core.index
 */
class synd_plugin_age extends SyndIndexExtension {
	/**
	 * Governs how fast the rank of older document decreses (0 < K1 <= 1)
	 *
	 * 0.999999 gives min resultant rank (K2) for ~4 week old documents.
	 *
	 * @param	float
	 */
	var $_K1 = 0.999999;
	
	/**
	 * The minimum resultant value of this weight (0 <= K2 <= 1)
	 * @param	float
	 */
	var $_K2 = 0.85;
	
	function initialize($index) {
		parent::initialize($index);
		$index->attachHook('query', array($this, '_callback_query'));
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

		$modified = $builder->join('synd_search_document','d',array('docid')).'.modified';
		$weight = "$this->_K2 + POWER($this->_K1, ".time()." - $modified) * ".(1 - $this->_K2);
		$builder->column($weight, 'WEIGHT', '*');

		$builder->popContext();
	}
}
