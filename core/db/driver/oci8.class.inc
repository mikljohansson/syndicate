<?php
/**
 * @package		synd.core.db
 * @filesource
 */
require_once 'core/db/Database.class.inc';

/**
 * Oracle database driver
 *
 * Uses the OCI interface for talking to Oracle. Automatic handling 
 * and storage of large objects (CLOBS, BLOBS, ...) are supported.
 *
 * @access		public
 * @package		synd.core.db
 */
class synd_database_oci8 extends DatabaseConnection {
	/**
	 * Last executed statement
	 */
	protected $_stmt = null;

	/**
	 * oci_execute() flags
	 */
	protected $_mode = OCI_COMMIT_ON_SUCCESS;

	/**
	 * Table keys
	 * @var		array
	 */
	protected $_keys = null;
	
	function _connect($uri, $args) {
		profiler_enter(__METHOD__);
		
		$handle = empty($args['persistent']) ? 
			oci_connect($uri['user'], $uri['pass'], $uri['host'], isset($args['charset']) ? $args['charset'] : null) : 
			oci_pconnect($uri['user'], $uri['pass'], $uri['host'], isset($args['charset']) ? $args['charset'] : null);
		
		if (!is_resource($handle)) {
			profiler_leave(__METHOD__);
			return self::raiseException(oci_error());
		}
		
		profiler_leave(__METHOD__);
		return $handle;
	}

	static function raiseException($message, $sql = null) {
		if (!is_string($message)) {
			if (is_resource($message))
				$message = oci_error($message);
			if (is_array($message))
				return parent::raiseException($message['message'], $message['sqltext']);
			return parent::raiseException('Unknown', $sql);
		}
		return parent::raiseException($message, $sql);
	}

	function close() {
		if (null != $this->_db)
			oci_close($this->_db);
		$this->_db = null;
	}

	function _begin($savepoint) {
		if (null != $savepoint)
			$this->query("SAVEPOINT $savepoint");
		else
			$this->_mode = OCI_DEFAULT;
	}

	function _commit($savepoint) {
		if (null == $savepoint) {
			$this->_mode = OCI_COMMIT_ON_SUCCESS;
			if (!oci_commit($this->_db))
				return self::raiseException($this->_db);
		}
	}

	function _rollback($savepoint) {
		if (null != $savepoint)
			$this->query("ROLLBACK TO $savepoint");
		else {
			$this->_mode = OCI_COMMIT_ON_SUCCESS;
			if (!oci_rollback($this->_db))
				return self::raiseException($this->_db);
		}
	}
	
	function query($sql, $offset = 0, $limit = null, $prefetch = 50) {
		profiler_enter(__METHOD__);

		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);
		if (false === ($this->_stmt = oci_parse($this->_db, $sql)) || false === oci_execute($this->_stmt, $this->_mode)) {
			profiler_leave(__METHOD__);
			self::raiseException($this->_db, $sql);
		}
		oci_set_prefetch($this->_stmt, (null!=$limit?$limit:$prefetch)+1);

		profiler_leave(__METHOD__);
		return new synd_result_oci8($this->_stmt);
	}

	function getAll($sql, $offset = 0, $limit = null, $prefetch = 50, $flags = 0) {
		profiler_enter(__METHOD__);

		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);
		if (false == ($stmt = oci_parse($this->_db, $sql))) {
			profiler_leave(__METHOD__);
			self::raiseException($this->_db, $sql);
		}
		
		oci_set_prefetch($stmt, (null!=$limit?$limit:$prefetch)+1);
		if (false === oci_execute($stmt, $this->_mode)) {
			profiler_leave(__METHOD__);
			self::raiseException($this->_db, $sql);
		}
		oci_fetch_all($stmt, $rows, 0, -1, OCI_ASSOC|OCI_FETCHSTATEMENT_BY_ROW);

		profiler_leave(__METHOD__);
		return $rows;
	}

	function getCol($sql, $col = 0, $offset = 0, $limit = null, $prefetch = 50, $flags = 0) {
		profiler_enter(__METHOD__);

		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);
		if (false == ($stmt = oci_parse($this->_db, $sql))) {
			profiler_leave(__METHOD__);
			self::raiseException($this->_db, $sql);
		}

		oci_set_prefetch($stmt, (null!=$limit?$limit:$prefetch)+1);
		if (false === oci_execute($stmt, $this->_mode)) {
			profiler_leave(__METHOD__);
			self::raiseException($this->_db, $sql);
		}
		oci_fetch_all($stmt, $rows, 0, -1, (is_numeric($col)?OCI_NUM:OCI_ASSOC)|OCI_FETCHSTATEMENT_BY_COLUMN);

		profiler_leave(__METHOD__);
		return $rows[$col];
	}

	function getAssoc($sql, $offset = 0, $limit = null, $prefetch = 50) {
		profiler_enter(__METHOD__);

		if (0 != $offset || null != $limit)
			$sql = $this->limit($sql, $offset, $limit);
		if (false == ($stmt = oci_parse($this->_db, $sql))) {
			profiler_leave(__METHOD__);
			self::raiseException($this->_db, $sql);
		}

		oci_set_prefetch($stmt, (null!=$limit?$limit:$prefetch)+1);
		if (false === oci_execute($stmt, $this->_mode)) {
			profiler_leave(__METHOD__);
			self::raiseException($this->_db, $sql);
		}
		oci_fetch_all($stmt, $rows, 0, -1, OCI_NUM|OCI_FETCHSTATEMENT_BY_COLUMN);

		profiler_leave(__METHOD__);
		return empty($rows[0]) ? array() : array_combine($rows[0], $rows[1]);
	}

	function getRow($sql) {
		profiler_enter(__METHOD__);

		if (false == ($stmt = oci_parse($this->_db, $sql)) || false === oci_execute($stmt, $this->_mode)) {
			profiler_leave(__METHOD__);
			self::raiseException($this->_db, $sql);
		}
		$row = oci_fetch_array($stmt, OCI_ASSOC|OCI_RETURN_NULLS|OCI_RETURN_LOBS);

		profiler_leave(__METHOD__);
		return $row;
	}

	function getOne($sql) {
		profiler_enter(__METHOD__);
		if (false == ($stmt = oci_parse($this->_db, $sql)) || false === oci_execute($stmt, $this->_mode)) {
			profiler_leave(__METHOD__);
			self::raiseException($this->_db, $sql);
		}
		$row = oci_fetch_array($stmt, OCI_NUM|OCI_RETURN_NULLS|OCI_RETURN_LOBS);

		profiler_leave(__METHOD__);
		return $row[0];
	}

	function update($table, $data) {
		if (null == ($key = $this->getTableKey($table, $data)))
			return $this->_stmt = null;

		list($prepared, $clobs) = $this->_quote($table, $data);
		if (empty($prepared))
			return $this->_stmt = null;

		$sql = "
			UPDATE $table
			SET	".$this->_update($prepared)."
			WHERE ".$this->_where($key);
		
		return $this->_execute($sql, array(), $clobs);
	}
	
	function insert($table, $data) {
		list($prepared, $clobs) = $this->_quote($table, $data);
		if (empty($prepared))
			return $this->_stmt = null;

		$sql = "
			INSERT INTO $table
			(".implode(", ", array_keys($prepared)).")
			VALUES
			(".implode(", ", $prepared).")";
		return $this->_execute($sql, array_diff(
			$this->getTableStructure($table)->getColumnNames(), 
			array_keys(array_filter($data,'strlen'))), $clobs);
	}

	/**
	 * Executes an SQL statement and loads bound columns and clobs
	 * @param	string	SQL statement
	 * @param	array	Undefined columns to read back
	 * @param	array	Clob descriptors
	 * @return	array
	 */
	private function _execute($sql, $undefined, $clobs) {
		$this->begin();

		// Add RETURNING statement for clobs and undefined columns
		$sql .= $this->_returning(array_merge(array_keys($clobs), $undefined));
	
		// Parse statement
		if (false === ($this->_stmt = oci_parse($this->_db, $sql))) {
			$this->rollback();
			return self::raiseException($this->_db, $sql);
		}

		// Bind undefined columns in query
		if (SyndLib::isError($undefinedVars = $this->_bindVariables($this->_stmt, $undefined))) {
			$this->rollback();
			return $undefinedVars;
		}

		// Bind clobs columns in query
		if (SyndLib::isError($clobVars = $this->_bindClobs($this->_stmt, array_keys($clobs)))) {
			$this->rollback();
			return $clobVars;
		}

		// Execute statement
		$ok = oci_execute($this->_stmt, $this->_mode);
		
		// Save and free clobs
		if ($ok && 1 == oci_num_rows($this->_stmt)) 
			$this->_saveClobs($clobVars, $clobs);
		$this->_freeDescriptors($clobVars);

		// Rollback on error
		if (false == $ok) {
			$this->rollback();
			return self::raiseException($this->_stmt, $sql);
		}

		$this->commit();
		// Dereference and return bound variables
		return $this->_freeVariables($undefinedVars);
	}

	private function _quote($table, $data) {
		$clobs = array();
		$result = array();
		$columns = $this->getTableStructure($table)->getColumns();
		
		foreach (array_keys($data) as $column) {
			if (isset($columns[$column])) {
				if (null === $data[$column] || false === $data[$column] || '' === $data[$column])
					$result[$column] = 'NULL';
				else if ($columns[$column] instanceof DatabaseLobColumn) {
					$clobs[$column] = $data[$column];
					$result[$column] = 'EMPTY_CLOB()';
				}
				else
					$result[$column] = $this->quote($data[$column]);
			}
		}
		
		return array($result, $clobs);
	}

	function load($table, $rows) {
		if (!count($rows))
			return;

		$sql = "
			INSERT INTO $table /*+ APPEND */
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

	function truncate($table) {
		$this->query("TRUNCATE TABLE $table DROP STORAGE");
	}

	/**
	 * Uses sequences to avoid locking
	 */
	function nextId($name) {
		$sequence = $this->getSequenceName($name);
		$sql = "SELECT $sequence.nextval FROM dual";

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
		return is_resource($this->_stmt) ? oci_num_rows($this->_stmt) : 0;
	}

	function isAlive() {
		try {
			return $this->getOne('SELECT 1 FROM dual');
		}
		catch (DatabaseException $e) {
			return false;
		}
	}
	
	function getTables() {
		return $this->getCol('SELECT u.table_name FROM user_tables u');
	}

	function getSequences() {
		return $this->getAssoc('SELECT u.sequence_name, u.last_number FROM user_sequences u');
	}

	function getTableKeys($table) {
		if (!isset($this->_keys[$table])) {
			$sql = "
				SELECT i.INDEX_NAME, c.COLUMN_NAME FROM user_indexes i, user_ind_columns c 
				WHERE 
					i.index_name = c.index_name AND
					i.index_type = 'NORMAL' AND 
					i.table_name = ".$this->quote(strtoupper($table))." AND 
					i.uniqueness = 'UNIQUE' AND
					NOT EXISTS (
						SELECT 1 FROM user_tab_columns tc
						WHERE 
							tc.table_name = i.table_name AND
							tc.column_name = c.column_name AND
							tc.nullable = 'Y')";
			foreach ($this->getAll($sql) as $row)
				$this->_keys[$table][$row['INDEX_NAME']][] = $row['COLUMN_NAME'];
		}
		return $this->_keys[$table];
	}
	
	function getTableStructure($table) {
		require_once 'core/db/TableMetadata.class.inc';
		$structure = new TableMetadata($table);
		
		$sql = "
			SELECT c.COLUMN_NAME, c.DATA_TYPE, c.DATA_LENGTH, c.DATA_PRECISION, c.DATA_SCALE, c.NULLABLE, c.DATA_DEFAULT
			FROM user_tab_columns c
			WHERE c.table_name = ".$this->quote(strtoupper($table))."
			ORDER BY c.column_id";
		$rows = $this->getAll($sql);
		
		foreach ($rows as $row) {
			$null = 'Y' == $row['NULLABLE'];
			if (null == $row['DATA_DEFAULT'] || 'NULL' == trim($row['DATA_DEFAULT']))
				$default = null;
			else if ("'" == substr($row['DATA_DEFAULT'],0,1) && "'" == substr($row['DATA_DEFAULT'],-1))
				$default = substr($row['DATA_DEFAULT'],1,strlen($row['DATA_DEFAULT'])-2);
			else
				$default = $row['DATA_DEFAULT'];
				
			switch (strtoupper($row['DATA_TYPE'])) {
				case 'CHAR':
				case 'NCHAR':
				case 'VARCHAR':
				case 'VARCHAR2':
				case 'NVARCHAR2':
					$column = new DatabaseStringColumn($this, $row['COLUMN_NAME'], $null, $default, $row['DATA_LENGTH'], true);
					break;
				case 'BLOB':
				case 'CLOB':
				case 'NCLOB':
				case 'BFILE':
				case 'LONG':
				case 'LONG RAW':
					$column = new DatabaseLobColumn($this, $row['COLUMN_NAME'], $null, $default, null, true);
					break;
				case 'FLOAT':
				case 'NUMBER':
					$column = new DatabaseNumberColumn($row['COLUMN_NAME'], $null, $default, $row['DATA_PRECISION'], $row['DATA_SCALE']);
					break;
				case 'DATE':
					$column = new DatabaseDatetimeColumn($row['COLUMN_NAME'], $null, $default, 'YYYY-MM-DD');
					break;
				case 'TIMESTAMP':
					$column = new DatabaseDatetimeColumn($row['Field'], $null, $default);
					break;
				default:
					continue;
			}

			$structure->addColumn($column);
		}
		
		$sql = "
			SELECT c.CONSTRAINT_NAME, c.R_CONSTRAINT_NAME FROM user_constraints c
			WHERE 
				c.table_name = ".$this->quote(strtoupper($table))." AND
				c.constraint_type = 'R'";
		$relations = $this->getAll($sql);
		
		foreach ($relations as $relation) {
			$sql = "
				SELECT c.column_name FROM user_cons_columns c
				WHERE c.constraint_name = ".$this->quote($relation['CONSTRAINT_NAME'])."
				ORDER BY c.position";
			$source = new DatabaseConstraint($relation['CONSTRAINT_NAME'], $table, $this->getCol($sql));
		
			$sql1 = "
				SELECT c.table_name FROM user_constraints c
				WHERE c.constraint_name = ".$this->quote($relation['R_CONSTRAINT_NAME']);
			
			$sql2 = "
				SELECT c.column_name FROM user_cons_columns c
				WHERE c.constraint_name = ".$this->quote($relation['R_CONSTRAINT_NAME'])."
				ORDER BY c.position";
			$target = new DatabaseConstraint($relation['R_CONSTRAINT_NAME'], $this->getOne($sql1), $this->getCol($sql2));
			
			$structure->addRelation($source, $target);
		}
		
		return $structure;
	}

	function limit($sql, $offset, $limit) {
		assert('null != $limit');
		if ($offset <= 0)
			return "SELECT t.* FROM ($sql) t WHERE ROWNUM <= ".($offset + $limit);
		
		$qsql = "SELECT * FROM ($sql) WHERE null = null";
		if (false == ($stmt = oci_parse($this->_db, $qsql)) || false === oci_execute($stmt, $this->_mode))
			return self::raiseException($this->_db, $qsql);

		$lsql = "
			SELECT ";
		
		for ($i=1, $c=oci_num_fields($stmt); $i<=$c; $i++) {
			if ($i > 1) $lsql .= ', ';
			$lsql .= oci_field_name($stmt, $i);
		}
		
		$lsql .= " 
			FROM (
				SELECT t1.*, ROWNUM synd_rownum 
				FROM ($sql) t1) t2
			WHERE 
				t2.synd_rownum BETWEEN ".($offset+1)." AND ".($offset+$limit);
		return $lsql;
	}

	function bitand($a, $b) {
		return "BITAND($a, $b)";
	}
	
	function unixtodate($expr) {
		return "TO_DATE('1970-01-01', 'YYYY-MM-DD') + NUMTODSINTERVAL($expr, 'SECOND')";
	}	

	function date($expr) {
		return "TO_CHAR($expr, 'YYYY-MM-DD')";
	}

	function rand() {
		return 'DBMS_RANDOM.VALUE';
	}

	private function _returning($columns) {
		if (!count($columns))
			return null;
		$cols = implode(', ', $columns);
		$vars = implode(', ', SyndLib::array_prepend($columns, ':'));
		return "\nRETURNING $cols INTO $vars";
	}
	
	private function _bindVariables($stmt, $columns) {
		$vars = array();
		foreach ($columns as $column) {
			if (!oci_bind_by_name($stmt, ':'.$column, $vars[$column], 32))
				return self::raiseException($stmt);
		}
		return $vars;
	}
	
	private function _freeVariables($vars) {
		$clone = array();
		foreach ($vars as $key => $value)
			$clone[$key] = $value;
		return $clone;
	}
	
	private function _bindClobs($stmt, $columns) {
		$vars = array();
		foreach ($columns as $column) {
			$vars[$column] = oci_new_descriptor($this->_db, OCI_D_LOB);
			if (!oci_bind_by_name($stmt, ':'.$column, $vars[$column], -1, OCI_B_CLOB))
				return self::raiseException($stmt);
		}
		return $vars;
	}

	private function _saveClobs($vars, $data) {
		foreach (array_keys($vars) as $column)
			$vars[$column]->save($data[$column]);
	}
	
	private function _freeDescriptors($descriptions) {
		foreach (array_keys($descriptions) as $key)
			$descriptions[$key]->free();
	}
}

/**
 * Oracle result set
 */
class synd_result_oci8 extends AbstractResultSet {
	/**
	 * @access	private
	 */
	var $_stmt = null;

	function __construct($stmt) {
		$this->_stmt = $stmt;
	}

	function fetchRow() {
		$row = oci_fetch_array($this->_stmt, OCI_ASSOC|OCI_RETURN_NULLS|OCI_RETURN_LOBS);
		return null !== $row ? $row : false;
	}

	function fetchInto(&$row) {
		$row = oci_fetch_array($this->_stmt, OCI_ASSOC|OCI_RETURN_NULLS|OCI_RETURN_LOBS);
		return null !== $row ? $row : false;
	}
}
