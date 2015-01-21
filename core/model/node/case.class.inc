<?php
require_once 'core/model/DomainObject.class.inc';

class synd_node_case extends AbstractDomainObject {
	/**
	 * Default storage
	 * @var		DomainStorage
	 */
	private static $_default = null;
	
	var $_string = null;

	function __construct($storage, $id) {
		$this->nodeId = $id;
		list(, $this->_string) = explode('.', $id, 2);
		parent::__construct($storage, array());
	}
	
	function nodeId() {
		if (null != $this->_string)
			return $this->classId().'.'.$this->_string;
		return null;
	}
	
	function objectId() {
		return $this->_string;
	}
	
	function __factory() {}
	
	/**
	 * Returns this instance (special case should not be cloned)
	 */
	function copy() {
		return $this;
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('append' == $perm || 'remove' == $perm)
			return true;
		return parent::hasPermission($user, $perm, $arg);
	}
	
	static function getDefaultStorage() {
		if (null == self::$_default)
			self::$_default = DomainStorageManager::getStorage(array('identity', 'static'));
		return self::$_default;
	}

	function getIndexDocument($index) {
		return $index->createFragment($this->toString());
	}

	function delete()	{}
	function save() 	{}
	function _save()	{}
	
	function toString() {
		return $this->_string;
	}
	
	function bubbleEvent($event)	{}
	function _node_onchange_child()	{}
}
