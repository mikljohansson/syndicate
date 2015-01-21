<?php
require_once 'core/model/type/collection.class.inc';

class synd_type_ole_collection extends synd_type_collection {
	var $_contents = null;
	var $_filtered = array();
	
	function __construct($id, $contents = null) {
		parent::__construct();
		$this->_contents = $contents;
		if (null === $this->_contents && isset($_REQUEST[$id]))
			$this->_contents = $_REQUEST[$id];
	}

	static function __instance($id) {
		if (null != ($instance = parent::__instance($id)))
			return $instance;
		list (,, $oid) = explode('.', $id, 3);
		return SyndType::factory('ole_collection', $oid);
	}
	
	function __sleep() {
		$vars = array_keys(get_object_vars($this));
		unset($vars[array_search('_filtered', $vars)]);
		return $vars;
	}

	function getCount() {
		return count((array)$this->_contents);
	}
	
	function getContents($offset = 0, $limit = null) {
		return SyndLib::getInstances(array_slice((array)$this->_contents, $offset, $limit ? $limit : count($this->_contents)));
	}

	function getFilteredCount($classes) {
		$hash = SyndLib::hash($classes);
		if (isset($this->_filtered[$hash])) 
			return $this->_filtered[$hash];
		
		$this->_filtered[$hash] = 0;
		foreach (array_keys($contents = $this->getContents()) as $key) {
			foreach ($classes as $class) {
				if ($contents[$key] instanceof $class)
					$this->_filtered[$hash]++;
			}
		}

		return $this->_filtered[$hash];
	}
	
	function getFilteredContents($classes) {
		foreach (array_keys($contents = $this->getContents()) as $key) {
			foreach ($classes as $class) {
				if ($contents[$key] instanceof $class)
					continue 2;
			}
			unset($contents[$key]);
		}
		return $contents;
	}
}
