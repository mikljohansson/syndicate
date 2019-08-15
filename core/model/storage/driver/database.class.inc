<?php
require_once 'core/model/storage/DomainStorage.class.inc';
require_once 'core/model/DatabaseEntity.class.inc';
require_once 'core/db/Database.class.inc';

/**
 * Database storage backend
 * @see			DatabaseEntity
 * @package		synd.core.model
 */
class synd_storage_database extends PersistentStorage {
	/**
	 * @access	protected
	 */
	var $_db = null;
	
	/**
	 * Dirty nodes waiting for flush
	 * @access	private
	 */
	var $_dirty = null;
	
	/**
	 * Deleted nodes waiting for flush
	 * @access	private
	 */
	var $_deleted = null;
	
	/**
	 * @access	private
	 */
	var $_exitHandler = false;

	/**
	 * Lists of ids requested for preload indexed by clsid
	 * @access	private
	 */
	var $_preload = null;

	/**
	 * Cached node queries
	 * @access	private
	 */
	var $_queries = null;
	
	/**
	 * Cached column default values
	 * @access	private
	 */
	var $_default = null;

	/**
	 * @param	Database	Database to load nodes from
	 */
	function __construct($dsn) {
		parent::__construct($dsn);
		$this->_db = DatabaseManager::getConnection(substr($dsn, strpos($dsn, ':')+1));
	}
	
	function getDatabase() {
		return $this->_db;
	}

	/**
	 * Returns the next id from a named sequence
	 * @uses	DatabaseConnection::nextId()
	 * @param	string	Name of sequence
	 * @return	integer
	 */
	function nextId($name) {
		return $this->_db->nextId($name);
	}
	
	/**
	 * Creates a hash from the database dsn
	 */
	function getNamespace() {
		return $this->_db->getNamespace();
	}

	function _getInstances($storage, $ids) {
		profiler_enter(__METHOD__);
		$result = array();
		$queries = array();
		
		foreach ($ids as $id) {
			if (null != ($clsid = substr($id, 0, strpos($id,'.')))) {
				if (!isset($queries[$clsid]))
					$queries[$clsid] = call_user_func(array(SyndNodeLib::loadClass($clsid), 'getEntityQuery'), $this);
				$queries[$clsid]->addIdentifier($id);
			}
		}
		
		foreach ($queries as $clsid => $query) {
			profiler_enter("Load '$clsid' instances");

			// Schedule preload requests
			if (!empty($this->_preload[$clsid])) {
				$query->addIdentifier($this->_preload[$clsid]);
				unset($this->_preload[$clsid]);
			}

			$class = SyndNodeLib::loadClass($clsid);
			foreach ($queries[$clsid] as $row) {
				if (null != ($id = $queries[$clsid]->getIdentifier($row)))
					$result[$id] = new $class($storage, $this->_db, $row);
			}

			profiler_leave("Load '$clsid' instances");
		}
		
		profiler_leave(__METHOD__);
		return $result;
	}
	
	function preload($id) {
		if (null != ($clsid = substr($id, 0, strpos($id,'.')))) 
			$this->_preload[$clsid][] = $id;
	}
	
	function _factory($storage, $clsid) {
		if (null != ($class = SyndNodeLib::loadClass($clsid)) && !isset($this->_queries[$clsid]))
			$this->_queries[$clsid] = call_user_func(array($class, 'getEntityQuery'), $this);
		
		if (isset($this->_queries[$clsid])) {
			if (!isset($this->_default[$clsid])) {
				$this->_default[$clsid] = array();
				foreach ($this->_queries[$clsid]->getTables() as $table)
					$this->_default[$clsid] = array_merge($this->_db->getTableStructure($table)->getDefaultValues(), $this->_default[$clsid]);
			}
			
			$node = new $class($storage, $this->_db, $this->_default[$clsid]);
			$node->__factory();
			return $node;
		}
		
		$node = null;
		return $node;
	}
	
	function rewrite($node) {
		$this->_dirty[$node->nodeId] = $node;
		if (isset($this->_deleted[$node->nodeId]))
			unset($this->_deleted[$node->nodeId]);
		
		if (!$this->_exitHandler) {
			$this->_exitHandler = true;
			SyndLib::attachHook('shutdown', array($this, '_callback_flush'));
		}
	}
	
	function checkout($node) {
		if (isset($this->_dirty[$node->nodeId]))
			unset($this->_dirty[$node->nodeId]);
	}
	
	function delete($node) {
		if (isset($this->_dirty[$node->nodeId]))
			unset($this->_dirty[$node->nodeId]);
		$this->_deleted[$node->nodeId] = $node;
		
		if (!$this->_exitHandler) {
			$this->_exitHandler = true;
			SyndLib::attachHook('shutdown', array($this, '_callback_flush'));
		}
	}
	
	/**
	 * All insert/update/delete queries is run within a single 
	 * transaction that gets rolled back in case of error.
	 */
	function flush() {
		profiler_enter(__METHOD__);
		$queries = array();
		$this->_db->begin();
		
		try {
			for ($i=0; !empty($this->_dirty) && $i<10; $i++) {
				foreach (array_keys($this->_dirty) as $id) {
					assert('isset($this->_dirty[$id]) && null !== $this->_dirty[$id]->nodeId');

					$node = $this->_dirty[$id];
					unset($this->_dirty[$id]);

					if (isset($queries[$clsid = substr($node->nodeId, 0, strpos($node->nodeId,'.'))]) || 
						null !== ($queries[$clsid] = call_user_func(array("synd_node_$clsid", 'getEntityQuery'), $this))) {
						if ($node->_entity_onpreflush()) {
							$this->_db->rollback();
							trigger_error('Rollback triggered by '.$node->nodeId.'->_entity_onpreflush() == true', E_USER_NOTICE);
							return false;
						}

						$data = $node->getEntityData();
						foreach ($queries[$clsid]->getTables() as $table) {
							$result = $this->_db->replace($table, $data);
							if (SyndLib::isError($result)) {
								$this->_db->rollback();
								trigger_error($result->getMessage(), E_USER_WARNING);
								return false;
							}

							if (is_array($result) && !empty($result)) {
								foreach ($result as $key => $value)
									$node->data[$key] = $value;
							}
						}

						if ($node->_entity_onpostflush()) {
							$this->_db->rollback();
							trigger_error('Rollback triggered by '.$node->nodeId.'->_entity_onpostflush() == true', E_USER_NOTICE);
							return false;
						}
					}
				}

				assert('$i < 5');
			}

			for ($i=0; !empty($this->_deleted) && $i<10; $i++) {
				foreach (array_keys($this->_deleted) as $id) {
					assert('isset($this->_deleted[$id]) && null !== $this->_deleted[$id]->nodeId');
					$node = $this->_deleted[$id];
					unset($this->_deleted[$id]);

					if ($node->_node_onpredelete()) {
						$this->_db->rollback();
						trigger_error('Rollback triggered by '.$node->nodeId.'->_node_onpredelete() == true', E_USER_NOTICE);
						return false;
					}

					if (isset($queries[$clsid = substr($node->nodeId, 0, strpos($node->nodeId,'.'))]) || 
						null !== ($queries[$clsid] = call_user_func(array("synd_node_$clsid", 'getEntityQuery'), $this))) {
						foreach ($queries[$clsid]->getTables() as $table) {
							if (SyndLib::isError($result = $this->_db->delete($table, $node->data))) {
								$this->_db->rollback();
								trigger_error($result->getMessage(), E_USER_WARNING);
								return false;
							}
						}
					}

					if ($node->_node_onpostdelete()) {
						$this->_db->rollback();
						trigger_error('Rollback triggered by '.$node->nodeId.'->_node_onpredelete() == true', E_USER_NOTICE);
						return false;
					}
				}

				assert('$i < 5');
			}
		}
		catch (Exception $e) {
			$this->_db->rollback();
			throw $e;
		}

		$this->_db->commit();
		profiler_leave(__METHOD__);
		return true;
	}

	/**
	 * @access	private
	 */
	function _callback_flush() {
		$this->flush();
	}
}

/**
 * Database entity query specification
 * @access		public
 * @package		synd.core.model
 */
interface EntityQuery extends DatabaseQuery {
	/**
	 * Adds a node id to be loaded in this batch
	 * @param	mixed	Id or array of ids to load
	 */
	function addIdentifier($id);

	/**
	 * Extracts the identifier given a data row
	 * @return	string
	 */
	function getIdentifier($data);

	/**
	 * Returns the primary key column
	 * @return	string
	 */
	function getPrimaryKey();
	
	/**
	 * Creates an entity collection from this query
	 * @return	DatabaseEntityCollection
	 */
	function getEntities();
}

/**
 * Relational database entity query
 * @package		synd.core.model
 */
class DatabaseEntityQuery implements EntityQuery {
	protected $_storage = null;
	protected $_db = null;
	protected $_column = null;
	protected $_pkey = null;
	
	private $_query = null;
	
	/**
	 * @param	Database	Domain storage run query against
	 * @param	string		The primary table to load from
	 * @param	string		The primary key to use
	 */
	function __construct(DomainStorage $storage, DatabaseQuery $query, $table, $column) {
		$this->_storage = $storage;
		$this->_query = $query;
		$this->_column = $column;
		$this->_pkey = null != ($t = $this->join($table)) ? $t.'.'.$column : $column;
	}

	function __clone() {
		$this->_query = clone $this->_query;
	}
	
	function getIterator($offset = 0, $limit = null) {
		return $this->_query->getIterator($offset, $limit);
	}
	
	function count() {
		return $this->_query->count();
	}
	
	function getCol($col = 0, $offset = 0, $limit = null) {
		return $this->_query->getCol($col, $offset, $limit);
	}
	
	function getOne() {
		return $this->_query->getOne();
	}

	function toString() {
		return $this->_query->toString();
	}
	
	function addIdentifier($id) {
		$this->_query->in($this->_pkey, $id);
	}
	
	function getIdentifier($data) {
		return $data[$this->_column];
	}
	
	function getPrimaryKey() {
		return $this->_pkey;
	}
	
	function getEntities() {
		return new DatabaseEntityCollection($this->_storage, $this);
	}
	
	function getTables() {
		return $this->_query->getTables();
	}
	
	function in($column, $value) {
		$this->_query->in($column, $value);
	}
	
	function join($table, $prefix = null, $keys = null) {
		return $this->_query->join($table, 
			null !== $prefix ? $prefix : DatabaseConnection::alias($table).'x', 
			null !== $keys ? $keys : array($this->_column));
	}

	function column($expression, $alias = null, $operator = null) {
		$this->_query->column($expression, $alias, $operator);
	}

	function where($column, $value = SYND_SQL_MISSING_PARAM) {
		$this->_query->where($column, $value);
	}

	function order($column, $ascending = true) {
		$this->_query->order($column, $ascending);
	}
}

/**
 * Uses a fixed clsid and separate oids to identify nodes
 * @package		synd.core.model
 */
class FixedEntityQuery implements EntityQuery {
	protected $_storage = null;
	protected $_query = null;
	protected $_clsid = null;
	
	/**
	 * @param	Database	Domain storage run query against
	 * @param	string		The contained entity query
	 * @param	string		The clsid to prepend to ids
	 */
	function __construct(DomainStorage $storage, EntityQuery $query, $clsid) {
		$this->_storage = $storage;
		$this->_query = $query;
		$this->_clsid = $clsid;
	}
	
	function __clone() {
		$this->_query = clone $this->_query;
	}
	
	function getIterator($offset = 0, $limit = null) {
		return $this->_query->getIterator($offset, $limit);
	}
	
	function count() {
		return $this->_query->count();
	}
	
	function getCol($col = 0, $offset = 0, $limit = null) {
		return $this->_query->getCol($col, $offset, $limit);
	}
	
	function getOne() {
		return $this->_query->getOne();
	}

	function addIdentifier($id) {
		if (is_scalar($id))
			$this->_query->addIdentifier(substr($id, strpos($id, '.')+1));
		else {
			foreach ($id as $key)
				$this->_query->addIdentifier(substr($key, strpos($key, '.')+1));
		}
	}
	
	function getIdentifier($data) {
		if (null != ($id = $this->_query->getIdentifier($data)))
			return $this->_clsid.'.'.$id;
		return null;
	}
	
	function getPrimaryKey() {
		return $this->_query->getPrimaryKey();
	}

	function getEntities() {
		return new DatabaseEntityCollection($this->_storage, $this);
	}

	function toString() {
		return $this->_query->toString();
	}
	
	function getTables() {
		return $this->_query->getTables();
	}
	
	function in($column, $value) {
		$this->_query->in($column, $value);
	}
	
	function join($table, $prefix = null, $keys = null) {
		return $this->_query->join($table, $prefix, $keys);
	}

	function column($expression, $alias = null, $operator = null) {
		$this->_query->column($expression, $alias, $operator);
	}

	function where($column, $value = SYND_SQL_MISSING_PARAM) {
		$this->_query->where($column, $value);
	}

	function order($column, $ascending = true) {
		$this->_query->order($column, $ascending);
	}
}
