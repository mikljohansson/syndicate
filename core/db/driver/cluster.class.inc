<?php
/**
 * @package		synd.core.db
 * @filesource
 */
require_once 'core/db/Database.class.inc';

/**
 * Database cluster driver
 * 
 * Provides load-balancing and automatic failover with transaction
 * replay over multiple backends. If the connection fails at any time
 * the driver switches to another backend. If the failure occur inside
 * a transaction it will be replayed to the new master.
 *
 * A cluster strategy object determine how load is spread and how 
 * failovers are handled. This enables users to implement their own
 * balancing and failover schemes.
 *
 * The cluster can consist of any mix of database drivers if need be.
 *
 * DSN format
 *
 * 	cluster:hotstandby://mysql://master1.example.com/db,mysql://master2.example.com/db;mysql://slave1.example.com/db,mysql://slave2.example.com/database
 * 	cluster://oci8://user:password@master.example.com;oci8://user:password@replica1.example.com,oci8://user:password@replica2.example.com
 *
 * Available balancing strategies are
 *
 *  balanced	(synd_cluster_balanced)
 *  hotstandby	(synd_cluster_hotstandby, typically mysql multi master replication)
 *
 * @access		public
 * @package		synd.core.db
 */
class synd_database_cluster implements Database {
	/**
	 * Strategy to determine master/slave order
	 */
	protected $_strategy = null;
	
	/**
	 * Log of method calls in this transaction.
	 * 
	 * When switching masters on transaction replay this log will
	 * be replayed on the new master.
	 *
	 * @var		array	array(array(method, args))
	 */
	protected $_log = array();
	
	static function factory($driver, $dsn) {
		if (false === ($i = strpos($dsn, '://')))
			throw new InvalidArgumentException('Malformed DSN');
		$class = $i > ($j = strpos($dsn, ':')) ? 'synd_cluster_'.substr($dsn, $j+1, $i-($j+1)) : 'synd_cluster_balanced';
		$strategy = call_user_func(array($class, 'factory'), $class, substr($dsn, $i+3));
		return new $driver($strategy);
	}
	
	function __construct(DatabaseClusterStrategy $strategy) {
		$this->_strategy = $strategy;
	}
	
	/**
	 * Tries to switch databases after an error.
	 * @access	private
	 * @return	bool	True on success
	 */
	function _nextConnection() {
		$slave = $this->_strategy->nextSlave();
		return !$slave->isNull();
	}
	
	/**
	 * Tries to switch masters and replay the current transaction.
	 * @access	protected
	 * @return	bool		True on success
	 */
	function _replayTransaction() {
		// Get hold of the failed master
		$failed = $this->_strategy->getMaster();

		// Try to select a new master
		$master = $this->_strategy->nextMaster();
		if ($master->isNull())
			return false;

		// Try to rollback the failed master
		$failed->_rollback();

		// Replay the transaction on new master
		foreach ($this->_log as $call) {
			$result = call_user_func_array(array($master, $call[0]), $call[1]);
			if (SyndLib::isError($result))
				return $master->isAlive() ? false : $this->_replayTransaction();
		}
		
		return true;
	}

	/**
	 * Invokes a method on the master.
	 *
	 * Will record the interaction and if failure occur try and
	 * switch masters and replay the transaction.
	 *
	 * @access	protected
	 * @param	string	Method to invoke on master
	 * @param	array	Arguments to send to method
	 * @return	mixed
	 */
	function _invoke($method, $args = array()) {
		// Get hold of master and execute call
		$master = $this->_strategy->getMaster();
		$result = call_user_func_array(array($master, $method), $args);
		
		// If call failed, try to switch masters and repeat
		if (SyndLib::isError($result)) {
			if (!$master->isAlive() && $this->_replayTransaction())
				return $this->_invoke($method, $args);
		}
		// Log this call if inside transaction
		else if ($this->_transactions)
			$this->_log[] = array($method, $args);
		
		return $result;
	}

	function getDSN() {
		return $this->_strategy->getCurrent()->getDSN();
	}

	function close() {
		return $this->_strategy->getCurrent()->close();
	}

	function isNull() {
		return false;
	}

	function getNamespace() {
		return $this->_strategy->getNamespace();	
	}
	
	function inTransaction() {
		return $this->_strategy->getCurrent()->inTransaction();
	}

	function begin($savepoint = null) {
		$this->_invoke(__FUNCTION__, array($savepoint));
	}
	
	function commit() {
		$this->_invoke(__FUNCTION__);
		$this->_log = array();
	}
	
	function rollback($savepoint = null) {
		$this->_invoke(__FUNCTION__, array($savepoint));
		$this->_log = array();
	}
	
	function createQuery() {
		return $this->_strategy->getCurrent()->createQuery();
	}
	
	function query($sql, $offset = 0, $limit = null, $prefetch = 50) {
		$result = $this->_invoke(__FUNCTION__, array($sql, $offset, $limit, $prefetch));
		if (!is_object($result) || SyndLib::isError($result))
			return $result;
		return new SyndDBClusterResult($this, $result);
	}

	function execute($query, $offset = 0, $limit = null, $prefetch = 50) {
		return $this->query($query->toString(), $offset, $limit, $prefetch);
	}

	function getAll($sql, $offset = 0, $limit = null, $prefetch = 50, $flags = 0) {
		$slave = $this->_strategy->getSlave();
		$result = $slave->getAll($sql, $offset, $limit, $prefetch, $flags);
		if (SyndLib::isError($result) && !$slave->isAlive() && $this->_nextConnection())
			return $this->getAll($sql, $offset, $limit, $prefetch, $flags);
		return $result;
	}
	
	function getCol($sql, $col = 0, $offset = 0, $limit = null, $prefetch = 50, $flags = 0) {
		$slave = $this->_strategy->getSlave();
		$result = $slave->getCol($sql, $col, $offset, $limit, $prefetch, $flags);
		if (SyndLib::isError($result) && !$slave->isAlive() && $this->_nextConnection())
			return $this->getCol($sql, $col, $offset, $limit, $prefetch, $flags);
		return $result;
	}
	
	function getAssoc($sql, $offset = 0, $limit = null, $prefetch = 50) {
		$slave = $this->_strategy->getSlave();
		$result = $slave->getAssoc($sql, $offset, $limit, $prefetch);
		if (SyndLib::isError($result) && !$slave->isAlive() && $this->_nextConnection())
			return $this->getAssoc($sql, $offset, $limit, $prefetch);
		return $result;
	}
	
	function getRow($sql) {
		$slave = $this->_strategy->getSlave();
		$result = $slave->getRow($sql);
		if (SyndLib::isError($result) && !$slave->isAlive() && $this->_nextConnection())
			return $this->getRow($sql);
		return $result;
	}
	
	function getOne($sql) {
		$slave = $this->_strategy->getSlave();
		$result = $slave->getOne($sql);
		if (SyndLib::isError($result) && !$slave->isAlive() && $this->_nextConnection())
			return $this->getOne($sql);
		return $result;
	}

	function replace($table, $data) {
		return $this->_invoke(__FUNCTION__, array($table, $data));
	}

	function update($table, $data) {
		return $this->_invoke(__FUNCTION__, array($table, $data));
	}

	function insert($table, $data)	{
		return $this->_invoke(__FUNCTION__, array($table, $data));
	}
	
	function load($table, $rows) {
		return $this->_invoke(__FUNCTION__, array($table, $rows));
	}
	
	function delete($schema, $data) {
		return $this->_invoke(__FUNCTION__, array($schema, $data));
	}

	function truncate($table) {
		return $this->_invoke(__FUNCTION__, array($table));
	}
	
	function deleteQuick($table, $where) {
		return $this->_invoke(__FUNCTION__, array($table, $where));
	}
	
	function nextId($name) {
		return $this->_invoke(__FUNCTION__, array($name));
	}

	function affectedRows()	{
		return $this->_strategy->getCurrent()->affectedRows();
	}
	
	function foundRows()	{
		return $this->_strategy->getCurrent()->foundRows();
	}
	
	function isAlive() {
		$slave = $this->_strategy->getSlave();
		return $slave->isAlive() && !$slave->isNull();
	}

	function getTables() {
		$slave = $this->_strategy->getSlave();
		return $slave->getTables();
	}

	function getSequences() {
		$slave = $this->_strategy->getSlave();
		return $slave->getSequences();
	}

	function getTableStructure($table) {
		$slave = $this->_strategy->getSlave();
		return $slave->getTableStructure($table);
	}

	function analyze($table) {
		return $this->_invoke(__FUNCTION__, array($table));
	}
	
	function optimize($table) {
		return $this->_invoke(__FUNCTION__, array($table));
	}
	
	function order($table, $order) {
		return $this->_invoke(__FUNCTION__, array($table, $order));
	}

	function quote($value) {
		return $this->_strategy->getCurrent()->quote($value);
	}

	function limit($sql, $offset, $limit) {
		return $this->_strategy->getCurrent()->limit($sql, $offset, $limit);
	}
	
	function concat($values) {
		return $this->_strategy->getCurrent()->concat($values);
	}
	
	function ternary($if, $then, $else) {
		return $this->_strategy->getCurrent()->ternary($if, $then, $else);
	}
	
	function bitand($a, $b) {
		return $this->_strategy->getCurrent()->bitand($a, $b);
	}
	
	function unixtodate($expr) {
		return $this->_strategy->getCurrent()->unixtodate($expr);
	}
	
	function date($expr) {
		return $this->_strategy->getCurrent()->date($expr);
	}
	
	function rand() {
		return $this->_strategy->getCurrent()->rand();
	}
	
}

/**
 * Cluster strategy. 
 *
 * A cluster strategy object determine how load is spread and how 
 * failovers are handled. 
 */
interface DatabaseClusterStrategy {
	/**
	 * Cluster strategy factory
	 * @param	string	Strategy class name
	 * @param	string	DSN
	 */
	static function factory($driver, $dsn);
	
	/**
	 * Returns a hash based on the master servers
	 * @see		DatabaseConnection::getNamespace()
	 * @return	string
	 */
	function getNamespace();

	/**
	 * Returns the current database connection
	 * @return	Database
	 */
	function getCurrent();

	/**
	 * Returns a master database connection or synd_database_null on failure.
	 * @return	Database
	 */
	function getMaster();

	/**
	 * Returns a slave database connection or synd_database_null on failure.
	 * @return	Database
	 */
	function getSlave();

	/**
	 * Switches to the next master if one exists.
	 *
	 * Removes the previous master from the list and tries to switch 
	 * to a new one.
	 *
	 * @return	Database
	 */
	function nextMaster();

	/**
	 * Switches to the next slave if one exists.
	 *
	 * Removes the previous slave from the list and tries to switch 
	 * to a new one.
	 *
	 * @return	Database
	 */
	function nextSlave();
}

/**
 * Reads and writes are spread across multiple masters and slaves
 *
 * Typically used with MySQL Cluster
 *
 * @access		public
 * @package		synd.core.db
 */
class synd_cluster_balanced implements DatabaseClusterStrategy {
	/**
	 * List of master servers
	 * @var		array
	 * @access	protected
	 */
	var $_masters = null;

	/**
	 * List of slave servers
	 * @var		array
	 * @access	protected
	 */
	var $_slaves = null;

	/**
	 * Current database connection
	 * @var		object	Database
	 * @access	protected
	 */
	var $_current = null;

	/**
	 * Is the current connection a master?
	 * @var		bool
	 * @access	protected
	 */
	var $_isMaster = false;

	static function factory($driver, $dsn) {
		list ($masters, $slaves) = explode(';', $dsn);
		return new $driver(
			array_map('rawurldecode', explode(',', $masters)), 
			array_map('rawurldecode', explode(',', $slaves)));
	}

	/**
	 * @param	array	List of master dsn's
	 * @param	array	List of slave dsn's
	 */
	function __construct($masters, $slaves) {
		$this->_masters = $masters;
		$this->_slaves = $slaves;
	}
	
	function getNamespace() {
		$servers = empty($this->_masters) ? $this->_slaves : $this->_masters;
		sort($servers);
		$value = null;
		
		foreach ($servers as $dsn) {
			if (is_array($dsn)) 
				$value .= SyndLib::hash($dsn);
			else {
				if (null != $value)
					$value .= ' ';
				$info = parse_url($dsn);
				$value .= $info['host'].'#'.(isset($info['port']) ? $info['port'] : null).'#'.(isset($info['path']) ? $info['path'] : null);
			}
		}
		
		return md5($value);
	}
	
	function getCurrent() {
		return $this->getSlave();
	}
	
	function getMaster() {
		if (null !== $this->_current && $this->_isMaster)
			return $this->_current;
		
		if (!empty($this->_masters)) {
			$dsn = $this->_masters[array_rand($this->_masters)];
			$db = DatabaseManager::getConnection($dsn);
			
			if ($db->isNull()) {
				$this->_setFailed($dsn);
				return $this->getMaster();
			}
			
			$this->_current = $db;
			$this->_isMaster = true;
			return $this->_current;
		}
		
		trigger_error('Error attempting failover, no more backends available in cluster', E_USER_WARNING);
		return DatabaseManager::getConnection('null://null');
	}
	
	function getSlave() {
		if (null !== $this->_current)
			return $this->_current;

		if (!empty($this->_slaves)) {
			$dsn = $this->_slaves[array_rand($this->_slaves)];
			$db = DatabaseManager::getConnection($dsn);
			
			if ($db->isNull()) {
				$this->_setFailed($dsn);
				return $this->getSlave();
			}
			
			$this->_current = $db;
			$this->_isMaster = false !== array_search($dsn, $this->_masters);
			return $this->_current;
		}

		return $this->getMaster();
	}
	
	function nextMaster() {
		if (null !== $this->_current) {
			if ($this->_current->inTransaction())
				@$this->_current->rollback();
			$this->_setFailed($this->_current->_dsn);
			unset($this->_current);
			$this->_current = null;
		}
		return $this->getMaster();
	}
	
	function nextSlave() {
		if (null !== $this->_current) {
			if ($this->_current->inTransaction())
				@$this->_current->rollback();
			$this->_setFailed($this->_current->_dsn);
			unset($this->_current);
			$this->_current = null;
		}
		return $this->getSlave();
	}
	
	/**
	 * Removes a db from the master/slave lists
	 * @access	protected
	 * @param	string	DSN of database to remove
	 */
	function _setFailed($dsn) {
		unset($this->_masters[array_search($dsn, $this->_masters)]);
		unset($this->_slaves[array_search($dsn, $this->_slaves)]);
	}
}

/**
 * Used with one master server and a number of hotstandby masters
 *
 * Typically multi-master replication such as MySQL Replication, 
 * Oracle Advanced Replication or Oracle Data Guard
 */
class synd_cluster_hotstandby extends synd_cluster_balanced {
	/**
	 * Returns a master database connection or synd_database_null on failure.
	 * @return	Database
	 */
	function getMaster() {
		if (null !== $this->_current && $this->_isMaster)
			return $this->_current;

		if (!empty($this->_masters)) {
			$dsn = reset($this->_masters);
			$db = DatabaseManager::getConnection($dsn);
			
			if ($db->isNull()) {
				$this->_setFailed($dsn);
				return $this->getMaster();
			}
			
			$this->_current = $db;
			$this->_isMaster = true;
			return $this->_current;
		}

		return DatabaseManager::getConnection('null://null');
	}
}

class SyndDBClusterResult extends AbstractResultSet {
	/**
	 * @access	private
	 */
	var $_db = null;

	/**
	 * Number of results to skip when replaying a query
	 * @access	private
	 */
	var $_skip = 0;

	/**
	 * Real query result
	 * @access	private
	 */
	var $_result = null;

	function SyndDBClusterResult($db, &$result) {
		$this->_db = $db;
		$this->_result = $result;
	}

	function fetchRow()	{
		if (!SyndLib::isError($row = $this->_result->fetchRow()))
			$this->_skip++;
		else if (!$this->_db->isAlive() && !SyndLib::isError($this->_replayTransaction()))
			return $this->fetchRow();
		return $row;
	}
	
	function fetchInto(&$row) {
		if (!SyndLib::isError($result = $this->_result->fetchInto($row)))
			$this->_skip++;
		else if (!$this->_db->isAlive() && !SyndLib::isError($this->_replayTransaction()))
			return $this->fetchInto($row);
		return $result;
	}
	
	/**
	 * @access	private
	 */
	function _replayTransaction() {
		$call = array_pop($this->_db->_log);
		if (!$this->_db->_replayTransaction())
			throw new DatabaseException('Could not replay transaction');
		
		$master = $this->_db->_strategy->getMaster();
		$result = call_user_func_array(array($master, $call[0]), $call[1]);
		if (SyndLib::isError($result))
			return $result;

		for ($i=0; $i<$this->_skip; $i++) {
			if (SyndLib::isError($skip = $result->fetchRow()))
				return $skip;
		}

		unset($this->_result);
		$this->_result = $result->_result;
		return true;
	}
}
