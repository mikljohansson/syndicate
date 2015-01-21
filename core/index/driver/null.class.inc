<?php
require_once 'core/index/AbstractIndex.class.inc';

class synd_index_null extends AbstractIndex {
	function getMatchSet(IndexQuery $query, $offset, $limit, $order = 'relevance') {
		return array();
	}

	function getMatchCount(IndexQuery $query) {
		return 0;
	}
}
