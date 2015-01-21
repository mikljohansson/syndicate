<?php
require_once 'core/Controller.class.inc';

/**
 * @abstract
 * @access		public
 * @package		synd.core.model
 */
class SyndType extends AbstractController {
	var $_oid  = null;
	var $_nid  = null;
	
	function __construct($oid = null) {
		$this->_oid = null != $oid ? $oid : md5(uniqid(''));
	}
	
	static function __factory($class, $args = null) {
		return call_user_func_array(array(new ReflectionClass($class), 'newInstance'), $args);
	}
	
	/**
	 * Instance loader callback
	 * @access	protected
	 * @param	string	Id of instance to retrieve
	 * @return	array
	 */
	static function __instance($id) {
		return null;
	}
	
	function __clone() {
		$this->_oid = md5(uniqid('type.'));
	}
	
	function delete() {}

	function classId() {
		return substr(get_class($this), 10);
	}
	
	function objectId() {
		return $this->_oid;
	}
	
	function nodeId() {
		return $this->classId().'.'.$this->objectId();
	}

	function id() {
		$id = 'type.'.$this->nodeId();
		if (null != $this->_nid && null !== ($node = SyndNodeLib::getInstance($this->_nid)))
			$id = 'mplex;'.$node->id().';'.$id;
		return $id;
	}
	
	function getInstanceId() {
		return 'type.'.$this->nodeId();
	}

	function isNull() {
		return false;
	}

	function toString() {
		return $this->nodeId();
	}

	function setParent($node) {
		$this->_nid = $node->nodeId;
	}
	
	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->_nid)))
			$parent = SyndNodeLib::getInstance('null.null');
		return $parent;
	}
	
	/**
	 * Returns an attribute. Attributes are inherited
	 * @see		DomainObject::getAttribute()
	 * @return	string	Returns null if attribute not found
	 */
	function getAttribute($key) {
		if (null !== ($parent = $this->getParent()) && !$parent->isNull())
			return $parent->getAttribute($key);
		return null;
	}
	
	function getHandler() {
		if (null != ($parent = $this->getParent()))
			return $parent->getHandler();
		return 'node';
	}

	function getSize() {
		return sizeof($this);
	}
	
	static function factory($clsid) {
		if (null == ($class = SyndType::loadClass($clsid))) 
			return null;

		$args = func_get_args();
		array_shift($args);
		
		return call_user_func(array($class, '__factory'), $class, $args);
	}
	
	static function loadClass($clsid) {
		if (class_exists("synd_type_$clsid", false) || 
			preg_match('/^[a-z0-9_]+$/', $clsid) && (include_once("core/model/type/$clsid.class.inc")))
			return "synd_type_$clsid";
		return null;
	}

	static function _callback_instance(&$result, $id) {
		if (0 === strpos($id, 'type.')) {
			SyndType::loadClass($class = substr($id, 5, strpos($id, '.', 5) - 5));
			return null !== ($result = call_user_func(array('synd_type_'.$class, '__instance'), $id));
		}
	}
}

SyndLib::attachHook('instance', array('SyndType', '_callback_instance'));