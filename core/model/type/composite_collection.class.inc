<?php
require_once 'core/model/type/collection.class.inc';

class synd_type_composite_collection extends synd_type_collection {
	var $_fragments = null;
	var $_count = null;
	var $_filtered = array();
	
	function __construct($fragments = array()) {
		parent::__construct();
		foreach ($fragments as $fragment)
			$this->appendCollection($fragment);
	}
	
	function __sleep() {
		$vars = array_keys(get_object_vars($this));
		unset($vars[array_search('_count', $vars)]);
		unset($vars[array_search('_filtered', $vars)]);
		return $vars;
	}

	function toString() {
		return SyndLib::translate('Composite');
	}

	function getFragments() {
		return (array)$this->_fragments;
	}

	function appendCollection($fragment) {
		if ('synd_type_composite_collection' == get_class($fragment))
			$this->_fragments = array_merge((array)$this->_fragments, (array)$fragment->_fragments);
		else
			$this->_fragments[] = $fragment;
		$this->_count = null;
	}

	function getCount() {
		if (null === $this->_count) {
			foreach (array_keys($this->_fragments) as $key) 
				$this->_count += $this->_fragments[$key]->getCount();
		}
		return $this->_count;
	}
	
	function getContents($offset = 0, $limit = null) {
		$children = array();
		
		foreach ($this->_fragments as $fragment) {
			if (null !== $limit && count($children) >= $limit)
				break;

			if ($offset < ($count = $fragment->getCount())) {
				$children = array_merge(
					$children, 
					$fragment->getContents($offset > 0 ? $offset : 0, null !== $limit ? $limit - count($children) : null));
			}

			$offset -= $count;
		}
		
		return $children;
	}

	function getFilteredCount($classes) {
		$hash = SyndLib::hash($classes);
		if (!isset($this->_filtered[$hash])) {
			$this->_filtered[$hash] = 0;
			foreach ($this->_fragments as $fragment)
				$this->_filtered[$hash] += $fragment->getFilteredCount($classes);
		}
		return $this->_filtered[$hash];
	}

	function getFilteredContents($classes) {
		$children = array();
		foreach (array_keys($this->_fragments) as $key) {
			$children = array_merge(
				$children, 
				$this->_fragments[$key]->getFilteredContents($classes));
		}		
		return $children;
	}
}
