<?php
/**
 * @package		synd.core.db
 * @filesource
 */
require_once 'core/db/Database.class.inc';

/**
 * MySQL database driver
 * @access		public
 * @package		synd.core.db
 */
class synd_database_mysql extends DatabaseConnection {
	/**
	 * Max allowed packet size
	 * @var		integer
	 * @access	private
	 */
	var $_maxAllowedPacket = null;
	
	/**
	 * Table keys
	 * @var		array
	 * @access	private
	 */
	var $_keys = null;

	/**
	 * Serial table columns
	 * @var		array
	 * @access	private
	 */
	var $_autoincrement = null;

	function _connect($uri, $args) {
		profiler_enter(__METHOD__);
		if ('unix(' == $uri['host'] && preg_match('/([^)]+)\)(.+)/', $uri['path'], $matches)) {
			$host = "localhost:{$matches[1]}";
			$uri['path'] = $matches[2];
		}
		else
			$host = isset($uri['port']) ? $uri['host'].':'.$uri['port'] : $uri['host'];
		
		$user = isset($uri['user']) ? $uri['user'] : null;
		$pass = isset($uri['pass']) ? $uri['pass'] : null;
		
		$handle = empty($args['persistent']) ? 
			mysql_connect($host, $user, $pass) : 
			mysql_pconnect($host, $user, $pass);
		
		if (!is_resource($handle)) {
			profiler_leave(__METHOD__);
			throw new DatabaseException(mysql_error());
		}
		
		if (isset($args['charset']))
			mysql_query("SET CHARACTER SET {$args['charset']}", $handle);
		
		if (isset($uri['path']) && !mysql_select_db(trim($uri['path'],'/'), $handle)) {
			profiler_leave(__METHOD__);
			throw new DatabaseException(mysql_error($handle));
		}
	
		profiler_leave(__METHOD__);
		return $handle;
	}
	
	function close() {
		if (null != $this->_db)
			mysql_close($this->_db);
		$this->_db = null;
	}

	function _begin($savepoint) {
		if (null != $savepoint)
			$this->query("SAVEPOINT $savepoint");
		else
			$this->query('SET AUTOCOMMIT=0');
	}

	function _commit($savepoint) {
		if (null != $savepoint)
			$this->query("RELEASE SAVEPOINT $savepoint");
		else {
			$this->query('COMMIT');
			$this->query('SET AUTOCOMMIT=1');
		}
	}

	function _rollback($savepoint) {
		if (null != $savepoint)
			$this->query("ROLLBACK TO $savepoint");
		else {
			$this->query('ROLLBACK');
			$this->query('SET AUTOCOMMIT=1');
		}
	}
	
	function query($sql, $offset = 0, $limit = null, $prefetch = 50) {
		profiler_enter(__METHOD__);

		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);
		if (false === ($result = mysql_query($sql, $this->_db))) {
			profiler_leave(__METHOD__);
			throw new DatabaseQueryException(mysql_error($this->_db), $sql);
		}

		profiler_leave(__METHOD__);
		if (is_bool($result))
			return $result;
		return new synd_result_mysql($result);
	}
	
	/**
	 * The SQL_CALC_FOUND_ROWS flag is supported on MySQL 4.0+
	 */
	function getAll($sql, $offset = 0, $limit = null, $prefetch = 50, $flags = 0) {
		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);
		if ($flags & SYND_SQL_CALC_FOUND_ROWS)
			$sql = preg_replace('/^\s*SELECT\s/i', '\0SQL_CALC_FOUND_ROWS ', $sql);
		if ($flags & SYND_SQL_HIGH_PRIORITY)
			$sql = preg_replace('/^\s*SELECT\s/i', '\0HIGH_PRIORITY ', $sql);
			
		if (is_resource($result = mysql_query($sql, $this->_db))) {
			$rows = array();
			while (false !== ($row = mysql_fetch_assoc($result)))
				$rows[] = $row;
			return $rows;
		}
		
		if (false === $result)
			throw new DatabaseQueryException(mysql_error($this->_db), $sql);
		return array();
	}
	
	/**
	 * The SQL_CALC_FOUND_ROWS flag is supported on MySQL 4.0+
	 */
	function getCol($sql, $col = 0, $offset = 0, $limit = null, $prefetch = 50, $flags = 0) {
		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);
		if ($flags & SYND_SQL_CALC_FOUND_ROWS)
			$sql = preg_replace('/^\s*SELECT\s/i', '\0SQL_CALC_FOUND_ROWS ', $sql);
		if ($flags & SYND_SQL_HIGH_PRIORITY)
			$sql = preg_replace('/^\s*SELECT\s/i', '\0HIGH_PRIORITY ', $sql);

		if (is_resource($result = mysql_query($sql, $this->_db))) {
			$rows = array();
			if (is_numeric($col)) {
				while (false !== ($row = mysql_fetch_row($result)))
					$rows[] = $row[$col];
			}
			else {
				while (false !== ($row = mysql_fetch_assoc($result)))
					$rows[] = $row[$col];
			}
			return $rows;
		}
		
		if (false === $result)
			throw new DatabaseQueryException(mysql_error($this->_db), $sql);
		return array();
	}
	
	function getAssoc($sql, $offset = 0, $limit = null, $prefetch = 50) {
		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);

		if (is_resource($result = mysql_query($sql, $this->_db))) {
			$rows = array();
			while (false !== ($row = mysql_fetch_row($result)))
				$rows[$row[0]] = $row[1];
			return $rows;
		}
		
		if (false === $result)
			throw new DatabaseQueryException(mysql_error($this->_db), $sql);
		return array();
	}

	function getRow($sql) {
		$sql = $this->limit($sql, 0, 1);
		if (is_resource($result = mysql_query($sql, $this->_db)))
			return mysql_fetch_assoc($result);
		if (false == $result)
			throw new DatabaseQueryException(mysql_error($this->_db), $sql);
		return array();
	}
	
	function getOne($sql) {
		$sql = $this->limit($sql, 0, 1);
		if (is_resource($result = mysql_query($sql, $this->_db))) {
			$row = mysql_fetch_row($result);
			return $row[0];
		}
		if (false == $result)
			throw new DatabaseQueryException(mysql_error($this->_db), $sql);
		return null;
	}
	
	function insert($table, $data) {
		profiler_enter(__METHOD__);
		if (!SyndLib::isError($result = parent::insert($table, $data)) && $this->affectedRows()) {
			if (!array_key_exists($table, (array)$this->_autoincrement)) {
				$cols = $this->getCol("SHOW COLUMNS FROM $table WHERE Extra = 'auto_increment'");
				$this->_autoincrement[$table] = reset($cols);
			}
			if (null != $this->_autoincrement[$table] && !isset($data[$this->_autoincrement[$table]]) &&
				!SyndLib::isError($id = $this->getOne('SELECT LAST_INSERT_ID()')))
				return array($this->_autoincrement[$table] => $id);
		}
		profiler_leave(__METHOD__);
		return $result;
	}

	/**
	 * Returns the max_allowed_packet system variable
	 */
	protected function getMaxAllowedPacket() {
		if (null === $this->_maxAllowedPacket && (
			SyndLib::isError($this->_maxAllowedPacket = $this->getOne('select @@max_allowed_packet')) ||
			null == $this->_maxAllowedPacket)) {
			trigger_error("Could not get '@@max_allowed_packet' system variable': ".
				$this->_maxAllowedPacket->getMessage(), E_USER_NOTICE);
			$this->_maxAllowedPacket = 1048576;
		}
		return $this->_maxAllowedPacket;
	}

	function load($table, $rows) {
		profiler_enter(__METHOD__);

		$packet = $this->getMaxAllowedPacket();
		$result = true;
		$keys = array_keys($rows);
		
		for ($i=0, $cnt=count($keys); $i<$cnt;) {
			/*
			 * InnoDB can't handle REPLACE DELAYED INTO; also in write heavy 
			 * environments use of the DELAYED directive can lead to deadlocks
			 * and similar condidions.
			 */
			$sql = "
				INSERT IGNORE INTO $table
				(".implode(',', array_keys(reset($rows))).")
				VALUES ";
			
			for (; $i<$cnt; $i++) {
				if (strlen($sql)+strlen($str = "\n(".implode(",", $rows[$keys[$i]])."),") > $packet)
					break;
				$sql .= $str;
			}
			
			if (false === ($result = mysql_query(rtrim($sql, ','), $this->_db))) {
				profiler_leave(__METHOD__);
				throw new DatabaseQueryException(mysql_error($this->_db), $sql);
			}
		}
		
		profiler_leave(__METHOD__);
		return is_bool($result) ? $result : true;
	}

	function truncate($table) {
		$this->query("TRUNCATE TABLE $table");
	}

	/**
	 * Deletes data without merging index leaves (DELETE QUICK)
	 *
	 * Using this might lead to unused holes in the index storage
	 * and wasted space, use optimize() to recover lost space.
	 * 
	 * @see		optimize()
	 */
	function deleteQuick($table, $where) {
		return $this->query("DELETE QUICK FROM $table WHERE $where");
	}
	
	/**
	 * Uses INSERT INTO; SELECT LAST_INSERT_ID(); DELETE FROM .. with InnoDB to avoid locking
	 */
	function nextId($name) {
		$sequence = $this->getSequenceName($name);
		$sql = "INSERT INTO $sequence VALUES (NULL)";

		try {
			$this->query($sql);
		}
		catch (DatabaseException $e) {
			try {
				$sql2 = "
					CREATE TABLE IF NOT EXISTS ".$this->getSequenceName($name)." (
						id bigint(20) unsigned NOT NULL auto_increment,
						PRIMARY KEY (id)
					) ENGINE=InnoDB DEFAULT CHARSET=latin1";
				$this->query($sql2);
			}
			catch (DatabaseException $e) {}
			$this->query($sql);
		}
		
		// Fetch and remove inserted sequence value
		$id = $this->getOne("SELECT LAST_INSERT_ID()");
		$sql = "DELETE FROM $sequence WHERE id = $id";
		$this->query($sql);

		return $id;
	}

	function affectedRows() {
		if (0 == ($rows = mysql_affected_rows($this->_db)))
			$rows = preg_match('/Rows matched: (\d*)/', mysql_info($this->_db), $match) ? $match[1] : 0;
		return $rows;
	}

	function foundRows() {
		return $this->getOne('SELECT FOUND_ROWS()');
	}

	function isAlive() {
		return !SyndLib::isError($result = $this->getOne('SELECT 1')) && $result;
	}
	
	function getTables() {
		return $this->getCol('SHOW TABLES');
	}

	function getSequences() {
		$sequences = array();
		$tables = $this->getCol("SHOW TABLES LIKE '%_seq'");
		
		foreach ($tables as $table) {
			$this->begin();
			if (!SyndLib::isError($this->query("INSERT INTO $table VALUES (null)")))
				$sequences[$table] = $this->getOne('SELECT LAST_INSERT_ID()');
			$this->rollback();
		}
		
		return $sequences;
	}

	function getTableKeys($table) {
		if (!isset($this->_keys[$table])) {
			$skip = null;
			$rows = $this->getAll("SHOW INDEX FROM $table");
			if (SyndLib::isError($rows))
				return array();
			
			foreach ($rows as $row) {
				if (!array_key_exists('Null', $row) && 'PRIMARY' != $row['Key_name'] || 
					array_key_exists('Null', $row) && !empty($row['Null'])) {
					unset($this->_keys[$table][$row['Key_name']]);
					$skip[$row['Key_name']] = true;
				}
				else if (empty($row['Non_unique']) && !isset($skip[$row['Key_name']]))
					$this->_keys[$table][$row['Key_name']][] = $row['Column_name'];
			}
		}
		return $this->_keys[$table];
	}
	
	function getTableStructure($table) {
		require_once 'core/db/TableMetadata.class.inc';
		$structure = new TableMetadata($table);
		
		$sql = "SHOW COLUMNS FROM $table";
		$rows = $this->getAll($sql);
		
		if (SyndLib::isError($rows))
			return $structure; 
		
		foreach ($rows as $row) {
			if (!preg_match('/(\w+)(?:\((\d+)(?:,(\d+))?\))?/', $row['Type'], $matches))
				continue;
				
			$null = 'YES' == $row['Null'];
			$default = null == $row['Default'] || 'null' == $row['Default'] ? null : $row['Default'];
			$unsigned = false !== strpos($row['Type'], ' unsigned');
			
			switch (strtoupper($matches[1])) {
				case 'CHAR':
				case 'VARCHAR':
					$column = new DatabaseStringColumn($this, $row['Field'], $null, $default, $matches[2], true);
					break;
				case 'TINYTEXT':
					$column = new DatabaseStringColumn($this, $row['Field'], $null, $default, 255, true);
					break;
				case 'TINYBLOB':
					$column = new DatabaseLobColumn($this, $row['Field'], $null, $default, 255, true);
					break;
				case 'TEXT':
				case 'BLOB':
					$column = new DatabaseLobColumn($this, $row['Field'], $null, $default, 65535, true);
					break;
				case 'MEDIUMTEXT':
				case 'MEDIUMBLOB':
					$column = new DatabaseLobColumn($this, $row['Field'], $null, $default, 16777215, true);
					break;
				case 'LONGTEXT':
				case 'LONGBLOB':
					$column = new DatabaseLobColumn($this, $row['Field'], $null, $default, null, true);
					break;
				case 'TINYINT':
					$column = new DatabaseIntegerColumn($row['Field'], $null, $default, 8, $unsigned);
					break;
				case 'SMALLINT':
					$column = new DatabaseIntegerColumn($row['Field'], $null, $default, 16, $unsigned);
					break;
				case 'MEDIUMINT':
					$column = new DatabaseIntegerColumn($row['Field'], $null, $default, 24, $unsigned);
					break;
				case 'INT':
					$column = new DatabaseIntegerColumn($row['Field'], $null, $default, 32, $unsigned);
					break;
				case 'BIGINT':
					$column = new DatabaseIntegerColumn($row['Field'], $null, $default, 64, $unsigned);
					break;
				case 'FLOAT':
				case 'DOUBLE':
					$column = new DatabaseFloatColumn($row['Field'], $null, $default, $unsigned);
					break;
				case 'DATE':
					$column = new DatabaseDatetimeColumn($row['Field'], $null, $default, 'YYYY-MM-DD');
					break;
				case 'TIME':
					$column = new DatabaseDatetimeColumn($row['Field'], $null, $default, 'HH:MM:SS');
					break;
				case 'DATETIME':
				case 'TIMESTAMP':
					$column = new DatabaseDatetimeColumn($row['Field'], $null, $default);
					break;
				case 'YEAR':
					$column = new DatabaseIntegerColumn($row['Field'], $null, $default, 4);
					break;
				default:
					continue;
			}

			$structure->addColumn($column);
		}

		$sql = "SHOW CREATE TABLE $table";
		$rows = $this->getAll($sql); $row = reset($rows); $statement = end($row);
		preg_match_all('/CONSTRAINT `(\w+)` FOREIGN KEY \(([^)]+)\) REFERENCES `(\w+)` \(([^)]+)\)/i', $statement, $matches);
		
		foreach ($matches[1] as $i => $name) {
			preg_match_all('/`(\w+)`/', $matches[2][$i], $cols);
			$source = new DatabaseConstraint($name, $table, $cols[1]);
			preg_match_all('/`(\w+)`/', $matches[4][$i], $cols);
			$target = new DatabaseConstraint($name, $matches[3][$i], $cols[1]);
			$structure->addRelation($source, $target);
		}
		
		return $structure;
	}

	function analyze($table) {
		profiler_enter(__METHOD__);
		$result = $this->query("ANALYZE TABLE $table");
		profiler_leave(__METHOD__);
		return $result;
	}

	function optimize($table) {
		profiler_enter(__METHOD__);
		$result = $this->query("OPTIMIZE TABLE $table");
		profiler_leave(__METHOD__);
		return $result;
	}

	function order($table, $order) {
		profiler_enter(__METHOD__);
		$sql = null;
		foreach ($order as $col) {
			if (is_bool($col))
				$sql .= $col ? ' ASC' : ' DESC';
			else {
				if (null != $sql)
					$sql .= ', ';
				$sql .= $col;
			}
		}
		$result = $this->query("ALTER TABLE $table ORDER BY $sql");
		profiler_leave(__METHOD__);
		return $result;
	}

	function quote($value) {
		if (is_string($value))
			return "'".mysql_real_escape_string($value, $this->_db)."'";
		return parent::quote($value);
	}

	function limit($sql, $offset, $limit) {
		assert('null != $limit');
		return $sql." \r\nLIMIT $offset, $limit";
	}

	function concat($vals) {
		if (count($vals) < 2)
			return reset($vals);
		return 'CONCAT('.implode(',',$vals).')';
	}

	function ternary($if, $then, $else) {
		return "IF($if, $then, $else)";
	}

	function unixtodate($expr) {
		return "FROM_UNIXTIME($expr)";
	}

	function date($expr) {
		return "DATE_FORMAT($expr, '%Y-%m-%d')";
	}
	
	function rand() {
		return 'RAND()';
	}
}

/**
 * MySQL result set
 */
class synd_result_mysql extends AbstractResultSet {
	function __construct($result) {
		$this->_result = $result;
	}

	function fetchRow() {
		return mysql_fetch_assoc($this->_result);
	}

	function fetchInto(&$row) {
		return $row = mysql_fetch_assoc($this->_result);
	}
}
