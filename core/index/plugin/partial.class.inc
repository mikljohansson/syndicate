<?php
require_once 'core/index/SyndIndexExtension.class.inc';

/**
 * Partial matching extension
 *
 * Uses a 't1.term LIKE 'foobar%' construct when matching terms instead 
 * of the default crc32 matching. This lowers the performance somewhat 
 * but for most (small-medium) collections this isn't significant.
 */
class synd_plugin_partial extends SyndIndexExtension {
	var $_minLength = 4;
	
	function initialize($index) {
		parent::initialize($index);
		$index->attachHook('query_visit_term', array($this, '_callback_query_visit_term'));
		$index->attachHook('query_branched_term', array($this, '_callback_query_branched_term'));
//		$index->attachHook('query_visit_null', array($this, '_callback_query_visit_null'));
	}
	
	function _callback_query_visit_term(&$result, $builder, $expression) {
		if (strlen($expression->toString()) >= $this->_minLength) {
			$t = $builder->allocate('synd_search_termindex','t',array('docid','termid'));
			$td = $builder->allocate('synd_search_term','td',array('termid'));
			$result = "$td.term LIKE ".$this->_index->_db->quote($expression->toString().'%');
			return true;
		}
	}
	
	function _callback_query_branched_term(&$result, $visitor, $expression) {
		if (strlen($expression->toString()) >= $this->_minLength)
			$result++;
	}
	
//	/**
//	 * Implements the null-query as "term = ''" instead of as 
//	 * "termid = 0" since MySQL < 5.0 is not able to use more than
//	 * one index per table (index_merge), this is about twice as slow.
//	 */
//	function _callback_query_visit_null(&$result, $builder, $expression) {
//		$t = $builder->allocate('synd_search_termindex','t',array('docid','termid'));
//		$td = $builder->allocate('synd_search_term','td',array('termid'));
//		$result = "$td.term = ''";
//		return true;
//	}
}
