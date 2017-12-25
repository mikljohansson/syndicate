<?php
/**
 * @package		synd.core.db
 * @filesource
 */
require_once 'core/db/Database.class.inc';

/**
 * PostgreSQL database driver
 * @access		public
 * @package		synd.core.db
 */
class synd_database_pgsql extends DatabaseConnection {
	/**
	 * Last query result
	 * @access	private
	 */
	var $_result = null;

	/**
	 * Table keys
	 * @var		array
	 * @access	private
	 */
	var $_keys = null;

	function _connect($uri, $args) {
		profiler_enter(__METHOD__);
		$dsn = null;
		if (isset($uri['host']))
			$dsn .= "host={$uri['host']} ";
		if (isset($uri['port']))
			$dsn .= "port={$uri['port']} ";
		if (isset($uri['path']))
			$dsn .= 'dbname='.trim($uri['path'],'/').' ';
		if (isset($uri['user']))
			$dsn .= "user={$uri['user']} ";
		if (isset($uri['pass']))
			$dsn .= "password={$uri['pass']} ";
		
		$handle = empty($args['persistent']) ? 
			pg_connect($dsn) : 
			pg_pconnect($dsn);

		if (!is_resource($handle)) {
			profiler_leave(__METHOD__);
			throw new DatabaseException(pg_last_error());
		}
		
		if (isset($args['charset']))
			pg_set_client_encoding($handle, $args['charset']);
		
		profiler_leave(__METHOD__);
		return $handle;
	}
	
	function close() {
		if (null != $this->_db)
			pg_close($this->_db);
		$this->_db = null;
	}
	
	function _begin($savepoint) {
		if (null != $savepoint)
			$this->query("SAVEPOINT $savepoint");
		else
			$this->query('BEGIN');
	}

	function _commit($savepoint) {
		if (null != $savepoint)
			$this->query("RELEASE SAVEPOINT $savepoint");
		else
			$this->query('COMMIT');
	}

	function _rollback($savepoint) {
		if (null != $savepoint)
			$this->query("ROLLBACK TO $savepoint");
		else
			$this->query('ROLLBACK');
	}
	
	function query($sql, $offset = 0, $limit = null, $prefetch = 50) {
		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);
		if (false === ($this->_result = @pg_query($this->_db, $sql))) 
			throw new DatabaseQueryException(pg_last_error($this->_db), $sql);
		return new synd_result_pgsql($this->_result);
	}

	function getAll($sql, $offset = 0, $limit = null, $prefetch = 50, $flags = 0) {
		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);
		if (!is_resource($result = @pg_query($this->_db, $sql)))
			throw new DatabaseQueryException(pg_last_error($this->_db), $sql);
		
		if (!is_array($rows = pg_fetch_all($result)))
			return array();
		for ($i=count($rows)-1; $i>=0; $i--)
			$rows[$i] = array_change_key_case($rows[$i], CASE_UPPER);
		return $rows;
	}

	function getCol($sql, $col = 0, $offset = 0, $limit = null, $prefetch = 50, $flags = 0) {
		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);
		if (!is_resource($result = @pg_query($this->_db, $sql)))
			throw new DatabaseQueryException(pg_last_error($this->_db), $sql);

		$rows = array();
		if (is_numeric($col)) {
			while (false !== ($row = pg_fetch_row($result)))
				$rows[] = $row[$col];
		}
		else {
			$col = strtolower($col);
			while (false !== ($row = pg_fetch_assoc($result)))
				$rows[] = $row[$col];
		}
		return $rows;
	}

	function getAssoc($sql, $offset = 0, $limit = null, $prefetch = 50) {
		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);
		if (!is_resource($result = @pg_query($this->_db, $sql)))
			throw new DatabaseQueryException(pg_last_error($this->_db), $sql);

		$rows = array();
		while (false !== ($row = pg_fetch_row($result)))
			$rows[$row[0]] = $row[1];
		return $rows;
	}
	
	function getRow($sql) {
		$sql = $this->limit($sql, 0, 1);
		if (!is_resource($result = @pg_query($this->_db, $sql)))
			throw new DatabaseQueryException(pg_last_error($this->_db), $sql);
		if (!is_array($row = pg_fetch_assoc($result)))
			return array();
		return array_change_key_case($row, CASE_UPPER);
	}
	
	function getOne($sql) {
		$sql = $this->limit($sql, 0, 1);
		if (!is_resource($result = @pg_query($this->_db, $sql)))
			throw new DatabaseQueryException(pg_last_error($this->_db), $sql);
		if (!is_array($row = pg_fetch_row($result)))
			return null;
		return $row[0];
	}

	function getTables() {
		return $this->getCol("
			SELECT c.relname FROM pg_class c
			WHERE 
				c.relname !~ '^(pg_|sql_)' AND 
				c.relkind = 'r';");
	}

	function getSequences() {
		return $this->getAssoc("
			SELECT c.relname, CURRVAL(c.relname) FROM pg_class c
			WHERE 
				c.relkind = 'S' AND 
				c.relnamespace IN (
					SELECT n.oid FROM pg_namespace n
					WHERE 
						n.nspname NOT LIKE 'pg_%' AND 
						n.nspname != 'information_schema');");
	}

	function getTableKeys($table) {
		if (!isset($this->_keys[$table])) {
			$sql = "
				SELECT a.attnum, UPPER(a.attname) FROM pg_attribute a
				WHERE 
					a.attrelid = '$table'::regclass AND
					a.attnum > 0 AND 
					a.attnotnull AND
					NOT a.attisdropped";
			$columns = $this->getAssoc($sql);

			$sql = "
				SELECT i.indkey FROM pg_index i
				WHERE i.indrelid = '$table'::regclass AND i.indisunique";
			foreach ($this->getCol($sql) as $indexes) {
				$numbers = array_flip(explode(' ', $indexes));
				if (count($numbers) == count($key = SyndLib::array_kintersect($columns, $numbers)))
					$this->_keys[$table][] = $key;
			}
		}
		return $this->_keys[$table];
	}
	
	function getTableStructure($table) {
		require_once 'core/db/TableMetadata.class.inc';
		$structure = new TableMetadata($table);
		
		$sql = "
			SELECT UPPER(a.attname) AS ATTNAME, a.ATTTYPMOD, a.ATTNOTNULL, t.TYPNAME, 
				pg_get_expr(d.adbin, d.adrelid) AS ADSRC,
				CASE a.atttypid
					WHEN 21 /*int2*/ THEN 16
					WHEN 23 /*int4*/ THEN 32
					WHEN 20 /*int8*/ THEN 64
					WHEN 1700 /*numeric*/ THEN
						CASE WHEN a.atttypmod = -1
						THEN null
						ELSE ((a.atttypmod - 4) >> 16) & 65535     -- calculate the precision
						END
					WHEN 700 /*float4*/ THEN 24 /*FLT_MANT_DIG*/
					WHEN 701 /*float8*/ THEN 53 /*DBL_MANT_DIG*/
					ELSE null
				END AS DATA_PRECISION,
				CASE 
					WHEN a.atttypid IN (21, 23, 20) THEN 0
					WHEN a.atttypid IN (1700) THEN            
						CASE 
							WHEN a.atttypmod = -1 THEN null       
							ELSE (a.atttypmod - 4) & 65535            -- calculate the scale  
						END
					ELSE null
				END AS DATA_SCALE
			FROM pg_type t, pg_attribute a
			   LEFT JOIN pg_attrdef d
			   ON (d.adrelid = a.attrelid AND d.adnum = a.attnum)
			WHERE
				 a.atttypid = t.oid AND
				 a.attrelid = '$table'::regclass AND
				 a.attnum > 0 AND NOT a.attisdropped
			ORDER BY a.attnum";
		$rows = $this->getAll($sql);
		
		foreach ($rows as $row) {
			$null = empty($row['ATTNOTNULL']);
			if (preg_match("/^'(.+)'::[\w ]+$/", $row['ADSRC'], $matches))
				$default = $matches[1];
			else if ('nextval(' == strtolower(substr($row['ADSRC'],0,8)))
				$default = null;
			else {
				switch (strtoupper($row['ADSRC'])) {
					case 'NULL':
						$default = null;
						break;
					case 'TRUE':
						$default = true;
						break;
					case 'FALSE':
						$default = false;
						break;
					default:
						$default = $row['ADSRC'];
				}
			}
			
			switch (strtoupper($row['TYPNAME'])) {
				case 'CHAR':
				case 'VARCHAR':
					if ($row['ATTTYPMOD'] != -1) {
						$column = new DatabaseStringColumn($row['ATTNAME'], $null, $default, $row['ATTTYPMOD']-4);
						break;
					}
					// Fall through to TEXT column
				case 'TEXT':
					$column = new DatabaseLobColumn($row['ATTNAME'], $null, $default, null, false);
					break;
				case 'INT2':
				case 'SMALLINT':
					$column = new DatabaseIntegerColumn($row['ATTNAME'], $null, $default, 16);
					break;
				case 'INT':
				case 'INT4':
				case 'INTEGER':
					$column = new DatabaseIntegerColumn($row['ATTNAME'], $null, $default, 32);
					break;
				case 'INT8':
				case 'BIGINT':
					$column = new DatabaseIntegerColumn($row['ATTNAME'], $null, $default, 64);
					break;
				case 'FLOAT4':
				case 'FLOAT8':
					$column = new DatabaseFloatColumn($row['ATTNAME'], $null, $default);
					break;
				case 'NUMERIC':
					$column = new DatabaseNumberColumn($row['ATTNAME'], $null, $default, $row['DATA_PRECISION'], $row['DATA_SCALE']);
					break;
				case 'DATE':
					$column = new DatabaseDatetimeColumn($row['ATTNAME'], $null, $default, 'YYYY-MM-DD');
					break;
				case 'TIME':
					$column = new DatabaseDatetimeColumn($row['ATTNAME'], $null, $default, 'HH:MM:SS');
					break;
				case 'TIMESTAMP':
					$column = new DatabaseDatetimeColumn($row['ATTNAME'], $null, $default);
					break;
				case 'BOOL':
					$column = new DatabaseDatetimeColumn($row['ATTNAME'], $null, $default);
					break;
				default:
					continue;
			}

			$structure->addColumn($column);
		}
		
		return $structure;
	}

	/**
	 * Autoincremented column (SERIAL type) readback requires OID's 
	 * to be enabled on the table in question.
	 */
	function insert($table, $data) {
		if (SyndLib::isError($result = parent::insert($table, $data)) || !$this->affectedRows() ||
			!count($undefined = array_diff($this->getTableStructure($table)->getColumnNames(), array_keys($data))))
			return $result;

		$sql = "
			SELECT ".implode(', ', $undefined)." 
			FROM $table
			WHERE oid = ".pg_last_oid($this->_result);
		return $this->getRow($sql);
	}

	function truncate($table) {
		$this->query("TRUNCATE TABLE $table");
	}
	
	function isAlive() {
		return !SyndLib::isError($result = $this->getOne('SELECT 1')) && $result;
	}
	
	/**
	 * Uses sequences to avoid locking
	 */
	function nextId($name) {
		$sequence = $this->getSequenceName($name);
		$sql = "SELECT NEXTVAL('$sequence')";

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

	function affectedRows() {
		if (is_resource($this->_result))
			return pg_affected_rows($this->_result);
		return 0;
	}

	function limit($sql, $offset, $limit) {
		assert('null != $limit');
		return $sql." \nLIMIT $limit OFFSET $offset";
	}

	function concat($vals) {
		return implode('||', $vals);
	}

        function unixtodate($expr) {
                return "TO_TIMESTAMP($expr)";
        }

        function date($expr) {
                return "TO_CHAR($expr, 'YYYY-MM-DD')";
        }

	function rand() {
		return 'RANDOM()';
	}

	function quote($value) {
		// NOTE: pg_escape_literal/pg_escape_string doesn't handle \0 bytes, rather they
		// will just truncate the string on \0. To properly insert \0 the column also needs
		// to use the bytea type. See https://www.postgresql.org/docs/9.0/static/datatype-binary.html
		if (is_string($value))
			return "E'".pg_escape_bytea($value)."'";
		if (is_bool($value))
			return $value ? 'TRUE' : 'FALSE';
		return parent::quote($value);
	}
}

/**
 * PostgreSQL result set
 */
class synd_result_pgsql extends AbstractResultSet {
	function __construct($result) {
		$this->_result = $result;
	}

	function fetchRow() {
		if (is_array($row = pg_fetch_assoc($this->_result)))
			return array_change_key_case($row, CASE_UPPER);
		return $row;
	}

	function fetchInto(&$row) {
		if (is_array($row = pg_fetch_assoc($this->_result)))
			$row = array_change_key_case($row, CASE_UPPER);
		return $row;
	}
}
