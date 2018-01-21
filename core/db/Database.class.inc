<?php
/**
 * @package		synd.core.db
 * @filesource
 */
require_once 'core/lib/SyndLib.class.inc';

/**
 * MySQL SQL_BIG_RESULT directive
 */
define('SYND_SQL_CALC_FOUND_ROWS', 1);

/**
 * MySQL HIGH_PRIORITY directive
 */
define('SYND_SQL_HIGH_PRIORITY', 2);

/**
 * Database connection registry
 */
abstract class DatabaseManager {
	protected static $_instances = array();
	
	/**
	 * Database connection factory
	 *
	 * To connect to a database cluster the DSN can be specified as 
	 * an array where index 0 is a list of master DSN's and index 1 
	 * is a list of slaves.
	 *
	 * <code>
	 * 'mysql://user:password@localhost/database?persistent=1'
	 * 'mysql://user@unix(/tmp/mysql.sock)/database',
	 * 'oci8://user:password@localhost/user'
	 * 
	 * array(
	 * 	array('mysql://user@master1.example.com/db', 'mysql://user@master2.example.com/db'),
	 *	array('mysql://user@slave1.example.com/db', 'mysql://user@slave2.example.com/db'))
	 * </code>
	 *
	 * @param	string		Database connection string
	 * @return	Database
	 */
	static function getConnection($dsn) {
		if (isset(self::$_instances[$dsn]))
			return self::$_instances[$dsn];
		$driver = false !== ($i = strpos($dsn, ':')) ? substr($dsn, 0, $i) : $dsn;
		$class = 'synd_database_'.$driver;
		require_once 'core/db/driver/'.$driver.'.class.inc';
		return self::$_instances[$dsn] = call_user_func_array(array($class, 'factory'), array($class, $dsn));
	}
}

interface Database {
	/**
	 * Database connection factory
	 * @param	string	Driver class name
	 * @param	string	DSN
	 * @return	Database
	 */
	static function factory($driver, $dsn);

	/**
	 * Returns the DSN of this database
	 * @see		DatabaseManager::getConnection()
	 * @return	mixed
	 */
	function getDSN();

	/**
	 * Close the database connection immediatly
	 */
	function close();
	
	/**
	 * Null object pattern
	 */
	function isNull();

	/**
	 * Returns a hash based on the host, port and database from the DSN
	 * @return	string
	 */
	function getNamespace();

	/**
	 * Checks for outstanding transaction
	 * @return	bool
	 */
	function inTransaction();

	/**
	 * Begins a new transaction
	 * @param	string	Savepoint to set
	 */
	function begin($savepoint = null);

	/**
	 * Commits the innermost transaction
	 */
	function commit();

	/**
	 * Rollbacks the innermost transaction
	 * @param	string	Savepoint to rollback to
	 */
	function rollback($savepoint = null);

	/**
	 * DatabaseQuery factory method
	 * @return	DatabaseQuery
	 */
	function createQuery();

	/**
	 * Executes a query.
	 *
	 * Returns a result set that can be queried for any result rows.
	 * The query may the read-write, ie UPDATE, INSERT, DELETE etc.
	 *
	 * @param	string	Sql query to execute
	 * @param	integer	Offset in result set to start from, default 0
	 * @param	integer	Max number of rows to return, default all
	 * @param	integer	Prefetch buffer optimization (Oracle)
	 * @return	ResultSet
	 */
	function query($sql, $offset = 0, $limit = null, $prefetch = 50);

	/**
	 * Use Database::createQuery() and DatabaseQuery::getIterator()
	 * @deprecated
	 */
	function execute($query, $offset = 0, $limit = null, $prefetch = 50);

	/**
	 * Fetches the entire result set.
	 * @param	string	Sql query to execute
	 * @param	integer	Offset in result set to start from, default 0
	 * @param	integer	Max number of rows to return, default all
	 * @param	integer	Prefetch buffer optimization (Oracle)
	 * @param	integer	Query execution flags (MySQL)
	 * @return	array
	 */
	function getAll($sql, $offset = 0, $limit = null, $prefetch = 50, $flags = 0);
	
	/**
	 * Fetches an entire column from the result set.
	 * @param	string	Sql query to execute
	 * @param	mixed	Column index to fetch, string or integer
	 * @param	integer	Offset in result set to start from, default 0
	 * @param	integer	Max number of rows to return, default all
	 * @param	integer	Prefetch buffer optimization (Oracle)
	 * @param	integer	Query execution flags (MySQL)
	 * @return	array
	 */
	function getCol($sql, $col = 0, $offset = 0, $limit = null, $prefetch = 50, $flags = 0);
	
	/**
	 * Fetches the first two cols of the result set as key => value pairs.
	 * @param	string	Sql query to execute
	 * @param	integer	Offset in result set to start from, default 0
	 * @param	integer	Max number of rows to return, default all
	 * @param	integer	Prefetch buffer optimization (Oracle)
	 * @return	array
	 */
	function getAssoc($sql, $offset = 0, $limit = null, $prefetch = 50);
	
	/**
	 * Fetches the first row from the result set.
	 * @param	string	Sql query to execute
	 * @return	array
	 */
	function getRow($sql);

	/**
	 * Fetches the first column of the first row from the result set.
	 * @param	string	Sql query to execute
	 * @return	string
	 */
	function getOne($sql);

	/**
	 * Replaces data in a table
	 *
	 * Can optionally return an array containing unitialized and 
	 * auto incremented columns read back from database
	 *
	 * @see		insert()
	 * @param	string	Table name
	 * @param	array	Data like array('column'=>'value')
	 * @return	mixed
	 */
	function replace($table, $data);

	/**
	 * Updates data in a table
	 * @param	string	Table name
	 * @param	array	Data like array('column'=>'value')
	 */
	function update($table, $data);

	/**
	 * Inserts data into a table
	 *
	 * Can optionally return an array containing unitialized and 
	 * auto incremented columns read back from database
	 *
	 * @param	string	Table name
	 * @param	array	Data like array('column'=>'value')
	 * @return	mixed
	 */
	function insert($table, $data);

	/**
	 * Inserts the supplied rows into a table.
	 *
	 * Multiple row insert, used to quickly load large chunks of rows 
	 * into a table. Note that values are not quoted automatically, you
	 * have to do it yourself if need be.
	 *
	 * @see		quote()
	 * @param	string	Table to load into
	 * @param	array	2D array of string indexed rows
	 */
	function load($table, $rows);

	/**
	 * Delete data from tables according to schema
	 * @param	array	Table to delete from
	 * @param	array	Data to delete
	 */
	function delete($table, $data);

	/**
	 * Clears an entire table
	 *
	 * Table with incoming foreign key references can not be 
	 * truncated, but must be cleared using 'DELETE FROM sometable'
	 *
	 * @param	string	Table to truncate
	 */
	function truncate($table);

	/**
	 * Quick delete without reclaiming table and index storage
	 * @param	string	Table to delete from
	 * @param	string	Where condition
	 */
	function deleteQuick($table, $where);

	/**
	 * Returns the next id from a named sequence
	 * @param	string	Name of sequence
	 * @return	integer
	 */
	function nextId($name);

	/**
	 * Returns the number of rows affected by the last UPDATE/INSERT query
	 * @return	integer
	 */
	function affectedRows();
	
	/**
	 * Returns the number of rows in the last query, without LIMIT applied
	 * @link	http://dev.mysql.com/doc/mysql/en/information-functions.html
	 * @return	integer	Rows or NULL if unsupported feature
	 */
	function foundRows();

	/**
	 * Pings the database to check connectivity
	 * @return	bool
	 */
	function isAlive();

	/**
	 * Returns tables in this database
	 * @return	array
	 */
	function getTables();

	/**
	 * Returns sequences and their current values
	 * @return	array
	 */
	function getSequences();

	/**
	 * Returns table meta information
	 * @param	string	Table to read
	 * @return	TableMetadata
	 */
	function getTableStructure($table);

	/**
	 * Analyze the key distribution of a table
	 * @param	string	Table to analyze
	 */
	function analyze($table);
	
	/**
	 * Rebuild the indexes on a table (costly)
	 * @param	string	Table to optimize
	 */
	function optimize($table);

	/**
	 * Reorder the internal row storage order.
	 * @param	string	Table to order
	 * @param	array	array('col1',false,'col2')
	 */
	function order($table, $order);

	/**
	 * Quotes a value so it can safely used in a query.
	 * @param	mixed	Value to quote
	 * @return	mixed
	 */
	function quote($value);

	/**
	 * Wrap binary string for insertion.
	 * @param	string	Binary string to wrap
	 * @return	string
	 */
	function wrapBinary($value);
	
	/**
	 * Unwrap binary string after reading.
	 * @param	string	String from database to unwrap
	 * @return	string
	 */
	function unwrapBinary($value);

	/**
	 * Returns an sql limited query
	 * @param	string	Query to rewrite
	 * @param	string	Result set offset
	 * @param	string	Result set limit
	 * @return	string
	 */
	function limit($sql, $offset, $limit);
	
	/**
	 * Returns an sql concat statement
	 * @param	array	Values to concatenate
	 * @return	string
	 */
	function concat($vals);
	
	/**
	 * IF-THEN-ELSE translation
	 * @param	string	If condition
	 * @param	string	Then expression
	 * @param	string	Else expression
	 * @return	string
	 */
	function ternary($if, $then, $else);

	/**
	 * Returns an sql bitwise and statement
	 * @param	string	Left-hand-side
	 * @param	string	Right-hand-side
	 * @return	string
	 */
	function bitand($a, $b);

	/**
	 * Converts a unix timestamp into a DATE
	 * @param	string	Column name or expression to convert
	 * @return	string
	 */
	function unixtodate($expr);
	
	/**
	 * Converts a DATE into a string as YYYY-MM-DD
	 * @param	string	Column name or expression to convert
	 * @return	string
	 */
	function date($expr);
	
	/**
	 * Generate a random value in the 0 <= v <= 1 range
	 * @return	string
	 */
	function rand();
}

/**
 * Database connection
 *
 * Queries sent to the get functions (getAll, getCol, ...) must be 
 * read-only, ie only SELECT's. Read-write queries should use the 
 * query(), load(), replace() or other read-write functions.
 *
 * @abstract
 * @access		public
 * @package		synd.core.db
 */
abstract class DatabaseConnection implements Database {
	/**
	 * Connection handle (autoinitialized using __get() override)
	 */
	//protected $_db;
	
	protected $_dsn = null;
	protected $_uri = null;
	protected $_args = null;
	
	private $_savepoints = null;

	static function factory($driver, $dsn) {
		$uri = parse_url($dsn);
		if (isset($uri['query']))
			parse_str($uri['query'], $args);
		else
			$args = null;
		return new $driver($dsn, $uri, $args);
	}
	
	function __construct($dsn, $uri, $args) {
		$this->_dsn = $dsn;
		$this->_uri = $uri;
		$this->_args = $args;
	}
	
	/**
	 * Autoconnects the database handle
	 */
	function __get($name) {
		if ('_db' == $name)
			return $this->_db = $this->_connect($this->_uri, $this->_args);
		return null;
	}
	
	/**
	 * @access	protected
	 * @return	mixed	Returns the database handle
	 */
	abstract function _connect($uri, $args);

	function __sleep() {
		$vars = array_keys(get_object_vars($this));
		if (false !== ($key = array_search('_db', $vars)))
			unset($vars[$key]);
		unset($vars[array_search('_savepoints', $vars)]);
		return $vars;
	}

	function getDSN() {
		return $this->_dsn;
	}

	function isNull() {
		return false;
	}

	function getNamespace() {
		return md5($this->_uri['host'].'#'.(isset($this->_uri['port']) ? $this->_uri['port'] : null).'#'.(isset($this->_uri['path']) ? $this->_uri['path'] : null));
	}
	
	function inTransaction() {
		return !empty($this->_savepoints);
	}

	function begin($savepoint = null) {
		if (!empty($this->_savepoints) && null == $savepoint)
			$savepoint = 'synd_savepoint_'.count($this->_savepoints);
		$this->_begin($savepoint);
		$this->_savepoints[] = $savepoint;
	}
	
	function commit() {
		$this->_commit(end($this->_savepoints));
		array_pop($this->_savepoints);
	}
	
	function rollback($savepoint = null) {
		if (!empty($this->_savepoints) && null == $savepoint)
			$savepoint = end($this->_savepoints);
		$this->_rollback($savepoint);
		while (!empty($this->_savepoints) && $savepoint != array_pop($this->_savepoints));
	}
	
	/**
	 * Low-level begin transaction
	 * @param	string	Savepoint to set
	 */
	abstract protected function _begin($savepoint);

	/**
	 * Low-level transaction commit
	 * @param	string	Savepoint to commit
	 */
	abstract protected function _commit($savepoint);

	/**
	 * Low-level transaction rollback
	 * @param	string	Savepoint to rollback to
	 */
	abstract protected function _rollback($savepoint);
	
	function createQuery() {
		return new synd_query_sql($this);
	}

	function execute($query, $offset = 0, $limit = null, $prefetch = 50) {
		return $this->query($query->toString(), $offset, $limit, $prefetch);
	}

	function replace($table, $data) {
		if (false === ($result = $this->update($table, $data)) || !$this->affectedRows())
			$result = $this->insert($table, $data);
		return $result;
	}
	
	function update($table, $data) {
		profiler_enter(__METHOD__);
		if (null == ($key = $this->getTableKey($table, $data))) {
			profiler_leave(__METHOD__);
			return false;
		}

		$sql = "
			UPDATE $table
			SET ";
		
		$i = 0;
		foreach ($this->getTableStructure($table)->getColumnNames() as $column) {
			if (array_key_exists($column, $data)) {
				if ($i++) $sql .= ', ';
				$sql .= $column.' = ';
				$sql .= $this->quote($data[$column]);
			}
		}
		
		$sql .= "
			WHERE ".$this->_where($key);
		$result = $this->query($sql);

		profiler_leave(__METHOD__);
		return $result;
	}
	
	function insert($table, $data) {
		if (!count($columns = array_intersect($this->getTableStructure($table)->getColumnNames(), array_keys($data))))
			return false;

		$sql = "
			INSERT INTO $table
			(".implode(', ', $columns).")
			VALUES
			(";

		$i = 0;
		foreach ($columns as $column) {
			if ($i++) $sql .= ', ';
			$sql .= $this->quote($data[$column]);
		}

		$sql .= ')';
		
		if (SyndLib::isError($result = $this->query($sql)))
			return $result;
	}

	function load($table, $rows) {
		if (!count($rows))
			return;

		$sql = "
			INSERT INTO $table
			(".implode(',',array_keys(reset($rows))).")
			VALUES ";

		$this->begin();
		foreach ($rows as $row) {
			$sql2 = $sql.'('.implode(', ',$row).')';
			
			if (SyndLib::isError($result = $this->query($sql2))) {
				$this->rollback();
				return $result;
			}
		}
		$this->commit();
		return true;
	}
	
	function delete($table, $data) {
		if (null == ($key = $this->getTableKey($table, $data))) {
			$this->query("DELETE $table WHERE NULL=NULL");
			return null;
		}
		$sql = "
			DELETE FROM $table
			WHERE ".$this->_where($key);
		if (SyndLib::isError($result = $this->query($sql)))
			return $result;
	}

	function truncate($table) {
		return $this->query("DELETE FROM $table");
	}

	function deleteQuick($table, $where) {
		return $this->query("DELETE FROM $table WHERE $where");
	}

	/**
	 * Returns the sequence table to use
	 * @param	string	Name of sequence
	 * @return	string
	 */
	protected function getSequenceName($name) {
		return str_replace('.', '_', $name).'_seq';
	}

	function affectedRows()	{return 0;}
	function foundRows() {return null;}
	
	/**
	 * Returns unique table keys
	 * @param	string	Table to read
	 * @return	array
	 */
	abstract function getTableKeys($table);

	/**
	 * Selectes a suitable candidate key.
	 * @uses	getTableKeys()
	 * @param	string	Table to read
	 * @param	array	Data to match key to
	 * @return	array
	 */
	function getTableKey($table, $data) {
		if (null != ($keys = $this->getTableKeys($table))) {
			$columns = array_keys($data);
			foreach ($keys as $keyColumns) {
				if (count($keyColumns) == count(array_intersect($keyColumns, $columns))) {
					$key = array();
					foreach ($keyColumns as $column) {
						if (null === $data[$column] || '' === $data[$column])
							continue 2;
						$key[$column] = $data[$column];
					}
					return $key;
				}
			}
		}
		return null;
	}

	function analyze($table) {}
	function optimize($table) {}
	function order($table, $order) {}
	
	/**
	 * Exception factory
	 * @param	string	Error message
	 * @param	string	Query to cause the exception
	 */
	static function raiseException($message, $sql = null) {
		if (null != $sql)
			throw new DatabaseQueryException($message, $sql);
		throw new DatabaseException($message);
	}

	/**
	 * Returns the alias assigned for a table.
	 * @return	string
	 */
	static function alias($table) {
		static $alias = array();
		if (!isset($alias[$table]))
			$alias[$table] = 't'.count($alias);
		return $alias[$table];
	}

	/**
	 * Generates a WHERE statement from an associative array
	 * @access	protected
	 * @param	array	Associative array of values
	 * @param	bool	Quote values before using them
	 * @return	string
	 */
	function _where($values, $quote = true) {
		foreach ($values as $key => $value) {
			if (null !== $value)
				$values[$key] = $key.' = '.($quote ? $this->quote($value) : $value);
			else
				$values[$key] = $key.' IS NULL';
		}
		return implode(' AND ', $values);
	}
	
	/**
	 * Generates a SET statement. Does NOT quote values.
	 * @access	protected
	 * @param	array	Associative array of values
	 * @return	string
	 */
	function _update($values) {
		foreach ($values as $key => $value)
			$values[$key] = "$key = $value";
		return implode(', ', $values);
	}
	
	function quote($value) {
		if (null === $value)
			return 'NULL';
		if (is_int($value) || is_float($value))
			return $value;
		if (is_array($value)) {
			foreach ($value as $key => $val)
				$value[$key] = $this->quote($val);
			return $value;
		}
		if (is_object($value))
			return $this->quote($this->wrapBinary(serialize($value)));
		return "'".strtr($value, array("'"=>"''","\0"=>"\\0"))."'";
	}

        function wrapBinary($value) {
		// Most databases handle NUL bytes in strings just fine, not PostgreSQL though
		return $value;
	}

        function unwrapBinary($value) {
		// Most databases handle NUL bytes in strings just fine, not PostgreSQL though
		return $value;
	}

	function limit($sql, $offset, $limit) {
		return $sql;
	}
	
	function concat($vals) {
		$sql = array_pop($vals);
		while (null != ($value = array_pop($vals)))
			$sql = "CONCAT($value,$sql)";
		return $sql;
	}
	
	function ternary($if, $then, $else) {
		return "CASE WHEN $if THEN $then ELSE $else END";
	}

	function bitand($a, $b) {
		return "$a & $b";
	}

	function rand() {
		return 0;
	}
}

/**
 * Database query interface
 * @see			DatabaseConnection::execute()
 * @access		public
 * @package		synd.core.db
 */
interface DatabaseQuery extends IteratorAggregate, Countable {
	/**
	 * Renders this query to a string
	 * @return	string
	 */
	function toString();

	/**
	 * Iterates over the result set
	 * @param	int		Offset to start iteration at
	 * @param	int		The number of objects to iterate
	 * @return	Iterator
	 */
	//function getIterator($offset = 0, $limit = null);

	/**
	 * Fetches an entire column from the result set.
	 * @param	mixed	Column index to fetch, string or integer
	 * @param	integer	Offset in result set to start from, default 0
	 * @param	integer	Max number of rows to return, default all
	 * @return	array
	 */
	function getCol($col = 0, $offset = 0, $limit = null);

	/**
	 * Fetches the first column of the first row from the result set.
	 * @return	string
	 */
	function getOne();

	/**
	 * Adds an mycolumn IN (...) constraint
	 * @param	string		The column to constrain
	 * @param	Traversable	The non-quoted value(s) to use
	 */
	function in($column, $value);

	/**
	 * Returns the tables involved in this query
	 * @return	array
	 */
	function getTables();
	
	/**
	 * Joins a table into query using the specified key
	 * @param	string	Table to join into query
	 * @param	string	Desired table alias prefix
	 * @param	array	Keys to join on
	 * @return	string	Table alias
	 */
	function join($table, $prefix = 't', $keys = null);
	
	/**
	 * Select a column into the result set
	 * @param	string	Column name or expression
	 */
	function column($expression);
	
	/**
	 * Adds a new filter to the WHERE clause
	 * @param	string	Column to restrict
	 * @param	mixed	Value to restrict to
	 */
	function where($column, $value = null);
	
	/**
	 * Sort the query
	 * @param	string	Column to sort on
	 * @param	bool	Sort ascending
	 */
	function order($column, $ascending = true);
}

/**
 * Database result set
 * @access		public
 * @package		synd.core.db
 */
interface ResultSet extends Iterator {
	/**
	 * Fetches a row from the result set. 
	 *
	 * Returns an array that corresponds to the fetched row, or false 
	 * if there are no more rows.
	 *
	 * @return	mixed
	 */
	function fetchRow();
	
	/**
	 * Fetches a row from the result set into the variable row
	 *
	 * The contents of $row will be overwritten. Returns an array 
	 * that corresponds to the fetched row, or false if there are 
	 * no more rows.
	 *
	 * @param	array	Will be overwritten with result
	 * @return	mixed
	 */
	function fetchInto(&$row);
}

/**
 * Implements the Iterator interface using ResultSet::fetchRow()
 */
abstract class AbstractResultSet implements ResultSet {
	private $_key = 0;
	private $_value = null;
	
	function rewind() {
		$this->_key = 0;
		$this->_value = null;
	}
	
	function current() {
		if (null === $this->_value)
			$this->_value = $this->fetchRow();
		return $this->_value;
	}
	
	function key() {
		return $this->_key;
	}
	
	function next() {
		$this->_key++;
		$this->_value = null;
	}
	
	function valid() {
		return (bool)$this->current();
	}
}

/**
 * Database exception
 * @access		public
 * @package		synd.core.db
 */
class DatabaseException extends RuntimeException {}

class DatabaseQueryException extends DatabaseException {
	protected $_sql = null;
	
	function __construct($message, $sql) {
		parent::__construct($message);
		$this->_sql = $sql;
	}

	function getDescription() {
		return $this->_sql;
	}
}
