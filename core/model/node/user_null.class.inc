<?php
require_once 'core/model/node/null.class.inc';
require_once 'core/lib/auth/User.class.inc';

class synd_node_user_null extends synd_node_null implements User {
	function toString()		{return '';}
	function __toString()	{return '';}
	function getLogin() {return '';}
	function getContact() {return '';}
	function getPhone() {return '';}
	function getEmail() {return '';}
	function getAddress() {return '';}
	function getSocialSecurityNumber() {return '';}
	
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
		return null;
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
