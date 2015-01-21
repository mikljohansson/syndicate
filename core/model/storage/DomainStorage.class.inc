<?php

/**
 * Domain storage registry
 * @access		public
 * @package		synd.core.model
 */
abstract class DomainStorageManager {
	protected static $_instances = array();
	
	/**
	 * Domain storage factory
	 * @param	array			Storage connection string
	 * @return	DomainStorage	Storage instance
	 */
	static function getStorage($dsns) {
		if (!isset(self::$_instances[$hash = md5(implode($dsns))])) {
			require_once 'core/model/storage/driver/null.class.inc';
			$storage = synd_storage_null::null();
			
			foreach (array_reverse($dsns) as $dsn) {
				$driver = false !== ($i = strpos($dsn, ':')) ? substr($dsn, 0, $i) : $dsn;
				require_once 'core/model/storage/driver/'.$driver.'.class.inc';
				$storage = call_user_func_array(array('synd_storage_'.$driver, 'getStorage'), array($dsn, $storage));
			}
			
			self::$_instances[$hash] = $storage;
		}

		return self::$_instances[$hash];
	}
}

/**
 * Domain storage interface
 * @access		public
 * @package		synd.core.model
 */
interface DomainStorage {
	/**
	 * Domain storage factory
	 * @param	string			DSN
	 * @param	DomainStorage	Next in chain-of-command
	 * @return	DomainStorage
	 */
	static function getStorage($dsn, DomainStorage $next);

	/**
	 * Returns the DSN of this storage chain
	 * @see		DomainStorageManager::getStorage()
	 * @return	array
	 */
	function getDSN();

	/**
	 * Returns the namespace of this storage
	 * @return	string
	 */
	function getNamespace();

	/**
	 * Retrieves a node from storage
	 * @param	string	Id of node
	 * @return	DomainObject
	 */
	function getInstance($id);

	/**
	 * Retrieves a list of nodes from storage
	 * @param	array	List of node ids
	 * @return	array
	 */
	function getInstances($ids);
	
	/**
	 * Load instances from storage
	 *
	 * The return value might include instances that was not 
	 * included in the list of ids to facilitate preloading.
	 * 
	 * @access	protected
	 * @param	DomainStorage	First storage in chain
	 * @param	array		List of ids to load
	 * @return	array
	 */
	function _getInstances($storage, $ids);
	
	/**
	 * Hints that a node will be requested later on (performance optimization)
	 * @param	string	The node id to preload
	 */
	function preload($id);

	/**
	 * Creates a new node residing in this storage
	 * @param	string	clsid to instantiate
	 * @return	DomainObject
	 */
	function factory($clsid);
	
	/**
	 * @access	protected
	 */
	function _factory($storage, $clsid);

	/**
	 * Clones a node
	 * @return	DomainObject
	 */
	function copy($node);
	
	/**
	 * Node needs to be written to persistent storage
	 * @param	INode
	 */
	function rewrite($node);
	
	/**
	 * @deprecated
	 */
	function refresh($node);

	/**
	 * Checkout node to temporary storage
	 * @param	INode
	 */
	function checkout($node);

	/**
	 * Removes a node from storage
	 * @param	INode
	 */
	function delete($node);

	/**
	 * Flushes changes to persistent storage
	 * @return	bool	Returns true on success, false on failure
	 */
	function flush();

	/**
	 * Returns the persistent storage at the end of the chain
	 * @return	DomainStorage
	 */
	function getPersistentStorage();

	/**
	 * Returns the database from the persistent storage
	 * @return	Database
	 */
	function getDatabase();
}

/**
 * Forwards to next in chain-of-command
 * @access		public
 * @package		synd.core.model
 */
abstract class ForwardingStorage implements DomainStorage {
	protected $_dsn = null;
	protected $_next = null;

	static function getStorage($dsn, DomainStorage $next) {
		$class = false !== ($i = strpos($dsn, ':')) ? 'synd_storage_'.substr($dsn, 0, $i) : 'synd_storage_'.$dsn;
		return new $class($dsn, $next);
	}
	
	function __construct($dsn, DomainStorage $next) {
		$this->_dsn = $dsn;
		$this->_next = $next;
	}

	function getDSN() {
		return array_merge(array($this->_dsn), $this->_next->getDSN());
	}
	
	function getNamespace() {
		return $this->_next->getNamespace();
	}
	
	function getInstance($id) {
		if (null != $id && null != ($instances = $this->_getInstances($this, array($id))) && isset($instances[$id]))
			return $instances[$id];
		return null;
	}

	function getInstances($ids) {
		$result = array();
		$instances = $this->_getInstances($this, $ids);
		
		foreach ($ids as $id) {
			if (isset($instances[$id]))
				$result[$id] = $instances[$id];
		}
		
		return $result;
	}

	function _getInstances($storage, $ids) {
		return $this->_next->_getInstances($storage, $ids);
	}
	
	function preload($id) {
		$this->_next->preload($id);
	}

	function factory($clsid) {
		return $this->_factory($this, $clsid);
	}

	function _factory($storage, $clsid) {
		return $this->_next->_factory($storage, $clsid);
	}

	function copy($node) {
		return $this->_next->copy($node);
	}

	function rewrite($node) {
		$this->_next->rewrite($node);
	}

	function refresh($node) {
		$this->_next->refresh($node);
	}

	function checkout($node) {
		$this->_next->checkout($node);
	}

	function delete($node) {
		$this->_next->delete($node);
	}
	
	function flush() {
		return $this->_next->flush();
	}

	function getPersistentStorage() {
		return $this->_next->getPersistentStorage();
	}

	function getDatabase() {
		return $this->_next->getDatabase();
	}
}

/**
 * End-of-chain persistent storage
 * @access		public
 * @package		synd.core.model
 */
abstract class PersistentStorage implements DomainStorage {
	protected $_dsn = null;
	
	static function getStorage($dsn, DomainStorage $next) {
		$class = false !== ($i = strpos($dsn, ':')) ? 'synd_storage_'.substr($dsn, 0, $i) : 'synd_storage_'.$dsn;
		return new $class($dsn);
	}
	
	function __construct($dsn) {
		$this->_dsn = $dsn;
	}

	function getDSN() {
		return array($this->_dsn);
	}
	
	function getInstance($id) {
		if (null != $id && null != ($instances = $this->_getInstances($this, array($id))) && isset($instances[$id]))
			return $instances[$id];
		return null;
	}

	function getInstances($ids) {
		$result = array();
		$instances = $this->_getInstances($this, $ids);
		
		foreach ($ids as $id) {
			if (isset($instances[$id]))
				$result[$id] = $instances[$id];
		}
		
		return $result;
	}

	function preload($id) {}

	function factory($clsid) {
		return $this->_factory($this, $clsid);
	}

	function _factory($storage, $clsid) {
		$class = 'synd_node_'.$clsid;
		return new $class($this, $storage);
	}
	
	function copy($node) {
		return clone $node;
	}
	
	function refresh($node) {}
	function checkout($node) {}

	function getPersistentStorage() {
		return $this;
	}
	
	/**
	 * Return a null database per default
	 */
	function getDatabase() {
		require_once 'core/db/Database.class.inc';
		return DatabaseManager::getConnection('null://null');
	}
}
