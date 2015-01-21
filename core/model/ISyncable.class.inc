<?php
require_once 'core/model/Collection.class.inc';

/**
 * Memento passed to/from syncable instances
 * @abstract
 * @access		public
 * @package		synd.core.model
 */
class ISyncableMemento {
	/**
	 * Returns a string representation of this memento
	 * @return	string
	 */
	function toString() {}
}

/**
 * Syncable interface for use with syncronization module
 * @abstract
 * @access		public
 * @package		synd.core.model
 */
class ISyncable {
	/**
	 * Returns the id of this instance
	 * @return	string
	 */
	function id() {}

	/**
	 * Checks if an operation is allowed on this instance
	 * @param	User	User to check permission for
	 * @param	string	Operation to check. {read,write,...}
	 * @return	bool
	 */
	function hasPermission($user, $perm) {}

	/**
	 * Returns a memento
	 * @return	ISyncableMemento
	 */
	function getMemento() {}

	/**
	 * Sets data from a memento
	 *
	 * Returns true on success, false on failure
	 *
	 * @param	ISyncableMemento
	 * @return	bool
	 */
	function setMemento($memento) {}

	/**
	 * Returns previous revisions of this instance
	 * @return	array	Array of ISyncable
	 */
	function getRevisions() {}
}

/**
 * Syncable collection
 * @abstract
 * @access		public
 * @package		synd.core.model
 */
class ISyncableCollection {
	/**
	 * Null object pattern
	 * return	bool
	 */
	function isNull() {assert(false);}

	/**
	 * Returns the unique name/hash of this collection
	 *
	 * This name can be used to instantiate the collection on other
	 * occasions via the 'syncronize' hook.
	 *
	 * @return	string
	 */
	function getName() {assert(false);}
	
	/**
	 * Returns an instance
	 * @param	string	The instance id
	 * @return	ISyncable
	 */
	function getInstance($id) {assert(false);}
	
	/**
	 * Returns interface supported by this collection
	 * @return	array
	 */
	function getCalendarCapabilities() {assert(false);}
	
	/**
	 * Returns the instances in this collection
	 *
	 * If the one or both of the optional timestamps are provided,
	 * only the instances modified between the two timestamps should 
	 * be returned.
	 *
	 * @param	integer	The last timestamp 
	 * @param	integer	The next timestamp
	 * @return	array
	 */
	function getContents($last = null, $next = null) {assert(false);}
	
	/**
	 * Appends an instance to this collection
	 *
	 * Returns the newly created instance on success, false on 
	 * failure.
	 *
	 * @param	ISyncableMemento
	 * @return	ISyncable
	 */
	function append($memento) {assert(false);}

	/**
	 * Removes an instance from this collection
	 *
	 * Returns the newly removed instance on success, false on 
	 * failure.
	 *
	 * @param	ISyncable
	 * @return	ISyncable
	 */
	function remove($instance) {assert(false);}
}

/**
 * Null collection
 */
class SyncableNullCollection extends ISyncableCollection {
	function isNull() {
		return true;
	}
	
	function getName() {
		return 'null';
	}
	
	function getInstance($id) {
		return null;
	}
	
	function getCalendarCapabilities() {
		return array();
	}
	
	function getContents($last = null, $next = null) {
		return array();
	}
	
	function append($memento) {
		return new NullSyncable();
	}
	
	function remove($instance) {
		return $instance;
	}
}

/**
 * Unauthorized case marker
 */
class SyncableUnauthorizedCollection extends SyncableNullCollection {}

class NullSyncable extends ISyncable {
	function id() {
		return null;
	}

	function hasPermission($user, $perm) {
		return true;
	}

	function getMemento() {
		return null;
	}

	function setMemento($memento) {
		return true;
	}

	function getRevisions() {
		return array();
	}
}
