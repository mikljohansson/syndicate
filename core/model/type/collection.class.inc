<?php
require_once 'core/model/SyndType.class.inc';
require_once 'core/model/Collection.class.inc';

abstract class synd_type_collection extends SyndType implements Collection {
	private static $_instances = array();
	private $_stored = false;
	
	function id() {
		if (!$this->_stored) {
			$this->_stored = true;
			self::$_instances[parent::id()] = $this;
			SyndLib::runHook('variable_set', 'collection', parent::id(), $this, 3600*24);
		}
		return parent::id();
	}
	
	function delete() {
		if ($this->_stored) {
			$this->_stored = false;
			unset(self::$_instances[parent::id()]);
			SyndLib::runHook('variable_set', 'collection', parent::id(), null);
		}
		parent::delete();
	}
	
	static function __instance($id) {
		if (isset(self::$_instances[$id]))
			return self::$_instances[$id];
		if (null !== ($collection = SyndLib::runHook('variable_get', 'collection', $id))) {
			self::$_instances[$id] = $collection;
			return $collection;
		}
		return parent::__instance($id);
	}
	
	/**
	 * The instanceid is is md5(uniqid('')) so no particular access 
	 * control is needed. If you know the id we can assume that you 
	 * have access to the collection.
	 */
	function _public_getCount() {
		return $this->getCount();
	}

	function _public_getContents($offset = 0, $limit = null) {
		return $this->getContents($offset, $limit);
	}
	
	function _public_getFilteredCount($classes) {
		return $this->getFilteredCount($classes);
	}
	
	function _public_getFilteredContents($classes) {
		return $this->getFilteredContents($classes);
	}
}
