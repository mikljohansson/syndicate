<?php
require_once 'core/model/type/collection.class.inc';

class synd_type_query_collection extends synd_type_collection {
	var $_storage = null;
	var $_sql = null;
	var $_count = null;
	var $_filtered = array();
	
	function __construct($storage, $sql) {
		parent::__construct();
		$this->_storage = $storage;
		$this->_sql = $sql;
	}
	
	function __sleep() {
		$vars = array_keys(get_object_vars($this));
		unset($vars[array_search('_count', $vars)]);
		unset($vars[array_search('_filtered', $vars)]);
		return $vars;
	}
	
	function toString() {
		return SyndLib::translate('Query');
	}

	function getCount() {
		if (null === $this->_count) {
			$sql = "SELECT COUNT(*) FROM ($this->_sql) t";
			$this->_count = $this->_storage->getDatabase()->getOne($sql);
		}
		return $this->_count;
	}
	
	function getContents($offset = 0, $limit = null) {
		return $this->_storage->getInstances($this->_storage->getDatabase()->getCol($this->_sql,0,$offset,$limit));
	}

	function getFilteredCount($classes) {
		$hash = SyndLib::hash($classes);
		if (!isset($this->_filtered[$hash]))
			$this->_filtered[$hash] = count($this->getFilteredContents($classes));
		return $this->_filtered[$hash];
	}

	function getFilteredContents($classes) {
		foreach (array_keys($nodes = $this->getContents()) as $key) {
			foreach ($classes as $class) {
				if ($nodes[$key] instanceof $class)
					continue 2;
			}
			unset($nodes[$key]);
		}
		return $nodes;
	}
}
