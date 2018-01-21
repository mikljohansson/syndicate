<?php
require_once 'core/model/DomainObject.class.inc';
require_once 'core/model/DatabaseEntityCollection.class.inc';
require_once 'core/db/Database.class.inc';

interface DatabaseEntity {
	/**
	 * Returns the query used when loading entities
	 * @param	DomainStorage			The storage to run the query
	 * @return	EntityQuery
	 */
	static function getEntityQuery(DomainStorage $storage);

	/**
	 * Returns the node id part after the clsid
	 * @return	string
	 */
	function objectId();

	/**
	 * Returns entity data ready for insertion into database
	 * @return	array
	 */
	function getEntityData();

	/**
	 * Runs before syncing to storage
	 *
	 * Returning true from this callback will rollback the running 
	 * transaction
	 *
	 * @return	bool
	 */
	function _entity_onpreflush();

	/**
	 * Runs after syncing to storage
	 *
	 * Returning true from this callback will rollback the running 
	 * transaction
	 *
	 * @return	bool
	 */
	function _entity_onpostflush();
}

/**
 * Database entity
 * @abstract
 */
abstract class AbstractDatabaseEntity extends AbstractDomainObject implements DatabaseEntity {
	/**
	 * Default storage
	 * @var		DomainStorage
	 */
	private static $_default = null;

	/**
	 * Database handle (autoinitialized using __get() override)
	 */
	//public $_db = null;
	protected $_dbdsn = null;

	protected $_isNew = false;

	function __construct($storage, $database, $data) {
		parent::__construct($storage, $data);
		$this->_db = $database;
	}

	function __factory() {
		parent::__factory();
		$this->_isNew = true;
	}

	/**
	 * Don't serialize the database connection
	 */
	function __sleep() {
		$this->_dbdsn = $this->_db->getDSN();
		$vars = parent::__sleep();
		unset($vars[array_search('_db', $vars)]);
		return $vars;
	}
	
	/**
	 * Autoconnects the storage handle
	 */
	function __get($name) {
		if ('_db' == $name)
			return $this->_db = DatabaseManager::getConnection($this->_dbdsn);
		return parent::__get($name);
	}

	function __clone() {
		parent::__clone();
		$this->_isNew = true;
	}
	
	function objectId() {
		return substr($this->nodeId, strpos($this->nodeId, '.')+1);
	}

	/**
	 * @deprecated
	 */
	function isNew() {
		return $this->_isNew;
	}

	/**
	 * Returns storage based on $synd_maindb
	 *
	 * The chain of command returned
	 *  1) IdentityStorage	- To keep references intact
	 *  2) CachedStorage	- To cache nodes
	 *  3) TemporaryStorage	- To store nodes until committed to database
	 *  4) DatabaseStorage	- Persistent node storage
	 */
	static function getDefaultStorage() {
		global $synd_maindb;
		$dsn = array('identity', 'temporary', 'cache', 'database:'.$synd_maindb->getDSN());
		$storage = DomainStorageManager::getStorage($dsn);
		return $storage;
	}

	/**
	 * Creates an aggregate collection
	 * @param	DomainStorage	Domain storage run query against
	 * @param	string			The primary table to load from
	 * @param	string			The primary key to use
	 * @return	EntityQuery
	 */
	protected static function createQuery(DomainStorage $storage, $table, $column = 'NODE_ID') {
		return new DatabaseEntityQuery($storage, $storage->getDatabase()->createQuery(), $table, $column);
	}

	/**
	 * Creates an aggregate collection
	 * @param	string	The class to aggregate
	 * @param	string	Column name to store parent node id
	 * @return	AggregateEntityCollection
	 */
	protected function aggregate($class, $column = 'PARENT_NODE_ID') {
		return new AggregateEntityCollection($this, $class, $column);
	}
	
	/**
	 * Creates an associate collection
	 * @param	string		The table to hold the association
	 * @param	string		The class to associate to
	 * @param	string		Column to hold the parent id
	 * @param	string		Column to hold the child id
	 * @param	callback	Pre-insert callback on the form "array myInsertFilter(array $row)"
	 * @return	AssociateEntityCollection
	 */
	protected function associate($table, $class = null, $pcol = 'PARENT_NODE_ID', $ccol = 'CHILD_NODE_ID', $prefilter = null) {
		return new AssociateEntityCollection($this, $table, $class, $pcol, $ccol, $prefilter);
	}

	/**
	 * Returns entity data ready for insertion into database
	 *
	 * Non-scalars (objects and arrays) contained in the data member 
	 * will automatically serialized. The corresponding unserialize(Database::unwrapBinary())
	 * should be implemented in the child classes __contruct method.
	 *
	 * @see		__construct()
	 * @return	array
	 */
	function getEntityData() {
		$data = array();
		foreach (array_keys($this->data) as $key) {
			$data[$key] = is_object($this->data[$key]) || is_array($this->data[$key]) ? 
				$this->_db->wrapBinary(serialize($this->data[$key])) : $this->data[$key];
		}
		return $data;
	}
	
	/**
	 * @uses	DatabaseConnection::getTableStructure()
	 */
	function validate($data) {
		$errors = parent::validate($data);
		$query = call_user_func(array(get_class($this),'getEntityQuery'), $this->_storage);
				
		foreach ($query->getTables() as $table) {
			$structure = $this->_db->getTableStructure($table);
			$errors = array_merge($errors, $structure->validate(array_merge($this->data, $data)));
		}
		
		return $errors;
	}

	/**
	 * Set this node dirty if it resides in database
	 */
	function _save() {
		if ($this->_isNew)
			$this->_node_onchange_internal();
		else
			$this->save();
	}

	function _entity_onpreflush() {
		return false;
	}

	function _entity_onpostflush() {
		$this->_isNew = false;
		$this->_node_onchange();
		return false;
	}
}

