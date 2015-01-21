<?php
require_once 'core/model/node/case.class.inc';
require_once 'core/lib/auth/User.class.inc';

class synd_node_user_case extends synd_node_case implements User {
	var $_email = null;

	function __toString() {
		return $this->toString().' ('.$this->getContact().')';
	}

	function getLogin() {
		if (preg_match('/([\w\-+\.\s\/]+)(@[\w\-]+\.[\w\-\.]+)?/', $this->toString(), $matches))
			return $matches[1];
		return null;
	}
	
	function getContact() {
		return $this->getEmail();
	}
	
	function getPhone() {return null;}
	function getAddress() {return '';}
	
	function getEmail() {
		if (null !== $this->_email)
			return $this->_email;
		if (preg_match('/[\w\-+\.]+@[\w\-]+\.[\w\-\.]+/', $this->toString(), $matches))
			return $matches[0];
		return null;
	}
	
	function setEmail($email) {
		$this->_email = $email;
	}
	
	function getSocialSecurityNumber() {
		return '';
	}

	function getOrganization() {
		return SyndNodeLib::getInstance('null.null');
	}
	
	function getRoles() {
		return array();
	}
	
	function isInheritedFrom($interface) {
		if ('User' == $interface)
			return true;
		return parent::isInheritedFrom($interface);
	}
	
	function getIdentifier() {
		return $this->getEmail();
	}
	
	function getChildren() {
		return new EmptyIteratorAggregate();
	}
	
	function validatePassword($password) {
		return false;
	}
	
	static function resolveLogin($login)	{return null;}
	static function resolveEmail($id)		{return null;}
	static function findIdentifier($id)		{return null;}
	static function findInstances($query)	{return null;}
}

