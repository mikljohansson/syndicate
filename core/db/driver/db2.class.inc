<?php
/**
 * @package		synd.core.db
 * @filesource
 */
require_once 'core/db/Database.class.inc';

/**
 * IBM DB2 database driver
 * @access		public
 * @package		synd.core.db
 */
class synd_database_db2 extends DatabaseConnection {
	/**
	 * Last executed statement
	 */
	protected $_stmt = null;

	function _connect($uri, $args) {
		profiler_enter(__METHOD__);
		
		$handle = empty($args['persistent']) ? 
			db2_connect($uri['host'], $uri['user'], $uri['pass']) : 
			db2_pconnect($uri['host'], $uri['user'], $uri['pass']);
		
		if (!is_resource($handle)) {
			profiler_leave(__METHOD__);
			throw new DatabaseException();
		}
		
		profiler_leave(__METHOD__);
		return $handle;
	}
	
	function close() {
		if (null != $this->_db)
			db2_close($this->_db);
		$this->_db = null;
	}

	function _begin($savepoint) {
		if (null != $savepoint)
			$this->query("SAVEPOINT $savepoint");
		else
			db2_autocommit($this->_db, false);
	}

	function _commit($savepoint) {
		if (null != $savepoint)
			$this->query("RELEASE SAVEPOINT $savepoint");
		else {
			$this->query('COMMIT');
			db2_autocommit($this->_db, true);
		}
	}

	function _rollback($savepoint) {
		if (null != $savepoint)
			$this->query("ROLLBACK TO SAVEPOINT $savepoint");
		else {
			$this->query('ROLLBACK');
			db2_autocommit($this->_db, true);
		}
	}
	
	function query($sql, $offset = 0, $limit = null, $prefetch = 50) {
		profiler_enter(__METHOD__);

		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);
		if (false === ($result = db2_exec($this->_db, $sql))) {
			profiler_leave(__METHOD__);
			throw new DatabaseQueryException(null, $sql);
		}

		profiler_leave(__METHOD__);
		return new synd_result_db2($result);
	}
	
	function getAll($sql, $offset = 0, $limit = null, $prefetch = 50, $flags = 0) {
		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);
		if (false === ($result = db2_exec($this->_db, $sql))) 
			throw new DatabaseQueryException(null, $sql);

		$rows = array();
		while (false !== ($row = db2_fetch_assoc($result)))
			$rows[] = $row;
		return $rows;
	}
	
	function getCol($sql, $col = 0, $offset = 0, $limit = null, $prefetch = 50, $flags = 0) {
		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);
		if (false === ($result = db2_exec($this->_db, $sql))) 
			throw new DatabaseQueryException(null, $sql);

		$rows = array();
		if (is_numeric($col)) {
			while (false !== ($row = db2_fetch_array($result)))
				$rows[] = $row[$col];
		}
		else {
			while (false !== ($row = db2_fetch_assoc($result)))
				$rows[] = $row[$col];
		}
		return $rows;
	}
	
	function getAssoc($sql, $offset = 0, $limit = null, $prefetch = 50) {
		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);
		if (false === ($result = db2_exec($this->_db, $sql))) 
			throw new DatabaseQueryException(null, $sql);

		$rows = array();
		while (false !== ($row = db2_fetch_array($result)))
			$rows[$row[0]] = $row[1];
		return $rows;
	}

	function getRow($sql) {
		$sql = $this->limit($sql, 0, 1);
		if (false === ($result = db2_exec($this->_db, $sql))) 
			throw new DatabaseQueryException(null, $sql);
		return db2_fetch_assoc($result);
	}
	
	function getOne($sql) {
		$sql = $this->limit($sql, 0, 1);
		if (false === ($result = db2_exec($this->_db, $sql))) 
			throw new DatabaseQueryException(null, $sql);
		if (false !== ($row = db2_fetch_array($result)))
			return $row[0];
		return null;
	}

	function update($table, $data) {
		if (null == ($key = $this->getTableKey($table, $data)))
			return $this->_stmt = null;
		profiler_enter(__METHOD__);
		
		$columns = $this->getTableStructure()->getColumnNames();
		$sql = "
			UPDATE $table SET ";
		$i = 0;
		
		foreach ($columns as $key => $column) {
			if (isset($data[$column])) {
				if ($i++) 
					$sql .= ',';
				$sql .= "
					$column = ?";
			}
		}
		
		$sql .= "
			WHERE ".$this->_where($key);
		$result = $this->_execute($sql, $columns, $data);
		
		profiler_leave(__METHOD__);
		return $result;
	}
	
	function insert($table, $data) {
		profiler_enter(__METHOD__);
		
		$columns = $this->getTableStructure()->getColumnNames();
		$sql = "
			INSERT INTO $table 
			(".implode(', ', $columns).")
			VALUES
			(".implode(', ', array_fill(0, count($columns), '?')).")";
		$result = $this->_execute($sql, $columns, $data, true);
		
		profiler_leave(__METHOD__);
		return $result;
	}
	
	protected function _execute($_sql, $_columns, $_data, $_out = false) {
		if (false === ($this->_stmt = db2_prepare($this->_db, $_sql)))
			throw new DatabaseQueryException(null, $_sql);
		
		extract($_data);
		$_unknown = array();
		$_result = null;
		$_i = 1;
		
		foreach ($_columns as $_column) {
			if (isset($_data[$_column]))
				db2_bind_param($this->_stmt, $_i++, $_column, DB2_PARAM_IN);
			else if ($_out) {
				db2_bind_param($this->_stmt, $_i++, $_column, DB2_PARAM_OUT);
				$_unknown[] = $_column;
			}
		}
		
		if (false === db2_execute($this->_stmt))
			throw new DatabaseQueryException(null, $_sql);
		
		return empty($_unknown) null : compact($_unknown);
	}

	/**
	 * Uses sequences to avoid locking
	 */
	function nextId($name) {
		$sequence = $this->getSequenceName($name);
		$sql = "SELECT NEXTVAL FOR $sequence FROM sysibm.sysdummy1";

		try {
			$id = $this->getOne($sql);
		}
		catch (DatabaseException $e) {
			try {
				$sql2 = "
					CREATE SEQUENCE ".$this->getSequenceName($name)." 
					CACHE 250";
				$this->query($sql2);
			}
			catch (DatabaseException $e) {}
			$id = $this->getOne($sql);
		}
		
		return $id;
	}

	function affectedRows()	{
		return is_resource($this->_stmt) ? db2_num_rows($this->_stmt) : 0;
	}

	function isAlive() {
		try {
			return $this->getOne('SELECT 1 FROM sysibm.sysdummy1');
		}
		catch (DatabaseException $e) {
			return false;
		}
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
					$column = new DatabaseStringColumn($row['Field'], $null, $default, $matches[2], true);
					break;
				case 'TINYTEXT':
					$column = new DatabaseStringColumn($row['Field'], $null, $default, 255, true);
					break;
				case 'TINYBLOB':
					$column = new DatabaseLobColumn($row['Field'], $null, $default, 255, true);
					break;
				case 'TEXT':
				case 'BLOB':
					$column = new DatabaseLobColumn($row['Field'], $null, $default, 65535, true);
					break;
				case 'MEDIUMTEXT':
				case 'MEDIUMBLOB':
					$column = new DatabaseLobColumn($row['Field'], $null, $default, 16777215, true);
					break;
				case 'LONGTEXT':
				case 'LONGBLOB':
					$column = new DatabaseLobColumn($row['Field'], $null, $default, null, true);
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

	function limit($sql, $offset, $limit) {
		sql.gsub!(/SELECT/i, 'SELECT B.* FROM (SELECT A.*, row_number() over () AS internal$rownum FROM (SELECT')
		sql << ") A ) B WHERE B.internal$rownum > #{offset} AND B.internal$rownum <= #{limit + offset}"
		
		assert('null != $limit');
		if ($offset <= 0)
			return $sql."\r\nFETCH FIRST $limit ROWS";

		$qsql = "SELECT * FROM ($sql) WHERE null = null";
		if (false === ($stmt = db2_exec($this->_db, $qsql)))
			return new DatabaseQueryException(null, $qsql);

		$lsql = "
			SELECT ";
		
		for ($i=0, $c=db2_num_fields($stmt); $i<$c; $i++) {
			if ($i) $lsql .= ', ';
			$lsql .= db2_field_name($stmt, $i);
		}
		
		$lsql .= " 
			FROM (
				SELECT t1.*, ROW_NUMBER() OVER () synd_rownum 
				FROM ($sql) t1) t2
			WHERE 
				t2.synd_rownum BETWEEN ".($offset+1)." AND ".($offset+$limit);
		return $lsql;
	}

	function bitand($a, $b) {
		return "BITAND($a, $b)";
	}
	
	function unixtodate($expr) {
		return "DATE('1970-01-01') + ($expr) SECONDS";
	}	

	function date($expr) {
		return "CHAR($expr, iso)";
	}
	
	function rand() {
		return 'RAND()';
	}
}

/**
 * IBM DB2 result set
 */
class synd_result_db2 extends AbstractResultSet {
	function __construct($result) {
		$this->_result = $result;
	}

	function fetchRow() {
		return db2_fetch_assoc($this->_result);
	}

	function fetchInto(&$row) {
		return $row = db2_fetch_assoc($this->_result);
	}
}
