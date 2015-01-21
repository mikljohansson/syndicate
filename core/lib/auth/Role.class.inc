<?php
require_once 'core/Instance.class.inc';

/**
 * Role interface for use with the framework
 * @access		public
 * @package		synd.core.model
 */
interface Role extends Instance {
	/**
	 * Role callback, must not recurse upwards in tree.
	 * @param	Instance	User or role to check authorization for
	 * @param	Instance	Subject to check authorization on
	 * @return	bool
	 */
	function hasAuthorization(Instance $user, Instance $subject);
	
	/**
	 * Returns the name
	 * @return	string
	 */
	function toString();
	
	/**
	 * Returns a short description
	 * @return	string
	 */
	function getDescription();
	
	/**
	 * Returns the public identifier for this role
	 * @return	string
	 */
	function getIdentifier();
	
	/**
	 * Returns child roles
	 * @return	IteratorAggregate
	 */
	function getChildren();

	/**
	 * Resolves a public identifier
	 * @param	string	The organisation id to resolve
	 * @return	Role	Returns null if none found
	 */
	static function findIdentifier($id);
	
	/**
	 * Attempts to find instances matching a query
	 * @param	string		Search query to match
	 * @return	Collection
	 */
	static function findInstances($query);
}

class NullRole implements Role {
	static $_instance = null;
	
	static function getInstance() {
		if (null == self::$_instance)
			self::$_instance = new self();
		return self::$_instance;
	}
	
	function id() {
		return null;
	}
	
	function hasAuthorization(Instance $user, Instance $subject) {
		return false;
	}
	
	function toString()						{return '';}
	function getDescription()				{return '';}
	function getIdentifier()				{return '';}
	function getChildren()					{return new EmptyIteratorAggregate();}

	static function findIdentifier($id)		{return null;}
	static function findInstances($query)	{return SyndType::factory('null_collection');}
}