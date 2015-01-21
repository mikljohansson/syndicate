<?php
require_once 'core/model/DatabaseEntity.class.inc';

class synd_node_unit_test extends AbstractDatabaseEntity {
	var $_ssn = null;
	var $_string = null;
	var $_attributes = null;
	
	function __factory() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_unit_test');
		parent::__factory();
	}

	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_unit_test');
		parent::__clone();
	}
	
	function __get($key) {
		if (null !== ($value = parent::__get($key)))
			return $value;
		return $this->$key;
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		$query = self::createQuery($storage, 'synd_unit_test');
		$query->join('synd_unit_test2');
		return $query;
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function isPermitted($op, $arg = null) {
		if ('read' == $op)
			return true;
		if ('get' == $op && '_string' == $arg)
			return true;
		if ('invoke' == $op && 'getdataarray' == $arg)
			return true;
		if ('invoke' == $op && 0 === strpos($arg, 'validator1'))
			return true;
		return parent::isPermitted($op, $arg);
	}

	function getParent() {
		if (null === ($parent = $this->_storage->getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}
	
	function setParent($parent) {
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
	}
	
	function getAttribute($key) {
		return isset($this->_attributes[$key]) ? $this->_attributes[$key] : parent::getAttribute($key);
	}
	
	function setAttribute($key, $value) {
		$this->_attributes[$key] = $value;
	}
	
	function setSocialSecurityNumber($ssn) {
		$this->_ssn = $ssn;
	}
	
	function getSocialSecurityNumber() {
		return $this->_ssn;
	}

	function toString() {
		return $this->data['INFO_HEAD'];
	}
	
	function _public_toString() {
		return $this->toString();
	}
	
	function _public_getSelf() {
		return $this;
	}
	
	function _public_echo($message) {
		return $message;
	}
	
	function _public_proxy($node) {
		return $node->toString();
	}

	function _public_getAuthenticatedUser() {
		global $synd_user;
		return $synd_user->id();
	}

	/** /////////////////////////////////////////
	 * http://validator.xmlrpc.com/ functions
	 */
	function validator1_arrayOfStructsTest($a) {
		$sum = 0;
		foreach ($a as $struct)
			$sum += $struct['curly'];
		return $sum;
	}

	function validator1_countTheEntities($str) {
		return array(
			'ctLeftAngleBrackets' => substr_count($str,'<'),
			'ctRightAngleBrackets' => substr_count($str,'>'),
			'ctAmpersands' => substr_count($str,'&'),
			'ctApostrophes' => substr_count($str,"'"),
			'ctQuotes' => substr_count($str,'"'));
	}
	
	function validator1_easyStructTest($a) {
		return array_sum($a);
	}
	
	function validator1_echoStructTest($a) {
		return $a;
	}

	function validator1_manyTypesTest($number, $boolean, $string, $double, $dateTime, $base64) {
		xmlrpc_set_type($dateTime, 'datetime');
		xmlrpc_set_type($base64, 'base64');
		return array($number, $boolean, $string, $double, $dateTime, $base64);
	}

	function validator1_moderateSizeArrayCheck($a) {
		return reset($a).end($a);
	}

	function validator1_nestedStructTest($a) {
		return array_sum($a['2000']['04']['01']);
	}

	function validator1_simpleStructReturnTest($num) {
		return array(
			'times10' => $num*10, 
			'times100' => $num*100, 
			'times1000' => $num*1000);
	}
	/////////////////////////////////////////////
}
