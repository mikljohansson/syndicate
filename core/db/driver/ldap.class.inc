<?php
/**
 * @package		synd.core.db
 * @filesource
 */
require_once 'core/db/Database.class.inc';
require_once 'core/db/query/ldap.class.inc';

/**
 * LDAP database driver
 * @access		public
 * @package		synd.core.db
 */
class synd_database_ldap extends DatabaseConnection {
	/**
	 * Base DN
	 * @var		string
	 * @access	protected
	 */
	var $_basedn = null;
	
	/**
	 * Last executed statement
	 * @var		resource
	 * @access	protected
	 */
	var $_stmt = null;

	function _connect($uri, $args) {
		profiler_enter(__METHOD__);
		$port = isset($uri['port']) ? $uri['port'] : null;
		$user = isset($uri['user']) ? $uri['user'] : null;
		$pass = isset($uri['pass']) ? $uri['pass'] : null;
		
		$handle = @ldap_connect($uri['host']);
		if (!is_resource($handle)) {
			profiler_leave(__METHOD__);
			throw new DatabaseException("Could not connect to host '{$uri['host']}'");
		}

		if ('ldap3' == $uri['scheme'] && !ldap_set_option($handle, LDAP_OPT_PROTOCOL_VERSION, 3)) {
			profiler_leave(__METHOD__);
			throw new DatabaseException(ldap_error($handle));
		}

		if (!ldap_bind($handle, $user, $pass)) {
			profiler_leave(__METHOD__);
			throw new DatabaseException(ldap_error($handle));
		}
	
		if (isset($uri['path']))
			$this->_basedn = trim($uri['path'], '/');

		profiler_leave(__METHOD__);
		return $handle;
	}
	
	function getHandle() {
		return $this->_db;
	}
	
	function close() {
		if (isset($this->_db))
			ldap_unbind($this->_db);
		unset($this->_db);
		$this->_stmt = null;
	}
	
	function _begin($savepoint)		{}
	function _commit($savepoint)	{}
	function _rollback($savepoint)	{}
	
	/**
	 * @access	private
	 */
	function _statement($filter, $offset, $limit, $basedn, $type) {
		$stmt = null;
		
		// Attempt to reconnect a corrupt handle (after a few thousand queries)
		for ($i=0; $i<2 && !is_resource($stmt); $i++) {
			switch ($type) {
				case synd_query_ldap::SELECT:
					$stmt = $offset > 0 || $limit > 0 ? 
						@ldap_list($this->_db, null != $basedn ? $basedn : $this->_basedn, $filter, array(), 0, $offset+$limit) :
						@ldap_list($this->_db, null != $basedn ? $basedn : $this->_basedn, $filter);
					break;

				case synd_query_ldap::SEARCH:
					$stmt = $offset > 0 || $limit > 0 ? 
						@ldap_search($this->_db, null != $basedn ? $basedn : $this->_basedn, $filter, array(), 0, $offset+$limit) :
						@ldap_search($this->_db, null != $basedn ? $basedn : $this->_basedn, $filter);
					break;

				default:
					throw new Exception("Unknown query type '$type'");
			}
			
			if (!is_resource($stmt))
				$this->close();
		}

		return $stmt;
	}
	
	function createQuery($type = synd_query_ldap::SEARCH) {
		return new synd_query_ldap($this, $type);
	}

	function query($filter, $offset = 0, $limit = null, $basedn = null, $type = synd_query_ldap::SELECT) {
		if (!is_resource($this->_stmt = $this->_statement($filter, $offset, $limit, $basedn, $type)))
			throw new DatabaseException(ldap_error($this->_db));
		return new synd_result_ldap($this, $this->_stmt, $offset);
	}
	
	function execute($query, $offset = 0, $limit = null, $prefetch = 50) {
		if (1 == count($basedns = $query->getTables()))
			return $this->query($query->toString($this), $offset, $limit, reset($basedns), synd_query_ldap::SEARCH);

		$primary = true;
		$rows = array();

		while (false !== ($basedn = array_shift($basedns))) {
			$result = $this->query($query->toString($this), 
				$primary ? $offset : 0, $primary ? $limit : null, 
				$basedn, synd_query_ldap::SEARCH);

			while (false !== ($data = $result->fetchRow())) {
				$key = $query->getIdentifier($data);
				if (isset($rows[$key]))
					$rows[$key] = array_merge($rows[$key], $data);
				else if ($primary)
					$rows[$key] = $data;
			}

			$primary = false;
		}
		
		return new synd_rowset_ldap($rows);
	}
	
	function getAll($filter, $offset = 0, $limit = null, $basedn = null, $type = synd_query_ldap::SELECT) {
		if (!is_resource($this->_stmt = $this->_statement($filter, $offset, $limit, $basedn, $type)))
			throw new DatabaseQueryException(ldap_error($this->_db), $filter);
		if (false == ($entry = ldap_first_entry($this->_db, $this->_stmt)))
			return array();
	
		for ($i=0; $i<$offset; $i++) {
			if (false == ($entry = ldap_next_entry($this->_db, $entry)))
				return array();
		}
		
		$rows = array();
		do {
			$ptr = null;
			if (false == ($attribute = ldap_first_attribute($this->_db, $entry, $ptr)))
				continue;

			$row = array();
			do {
				if (false == ($values = ldap_get_values($this->_db, $entry, $attribute)))
					throw new DatabaseQueryException(ldap_error($this->_db), $filter);
				
				if (count($values) <= 2)
					$row[$attribute] = reset($values);
				else {
					unset($values['count']);
					$row[$attribute] = $values;
				}
			} while (false != ($attribute = ldap_next_attribute($this->_db, $entry, $ptr)));
	
			if (!isset($row['dn']))
				$row['dn'] = ldap_get_dn($this->_db, $entry);
			
			$rows[] = $row;
		} while (false != ($entry = ldap_next_entry($this->_db, $entry)));
		
		return $rows;
	}

	function getCol($filter, $col = 0, $offset = 0, $limit = null, $basedn = null, $type = synd_query_ldap::SELECT) {
		if (!is_resource($this->_stmt = $this->_statement($filter, $offset, $limit, $basedn, $type)))
			throw new DatabaseQueryException(ldap_error($this->_db), $filter);
		if (false == ($entry = ldap_first_entry($this->_db, $this->_stmt)))
			return array();
	
		for ($i=0; $i<$offset; $i++) {
			if (false == ($entry = ldap_next_entry($this->_db, $entry)))
				return array();
		}
		
		$rows = array();
		do {
			if ('dn' === $col)
				$rows[] = ldap_get_dn($this->_db, $entry);
			else {
				$ptr = null;
				if (false == ($attribute = ldap_first_attribute($this->_db, $entry, $ptr)))
					continue;
				if (is_numeric($col)) {
					for ($i=0; $i<$col; $i++) {
						if (false == ($attribute = ldap_next_attribute($this->_db, $entry, $ptr)))
							continue 2;
					}
				}
				else {
					while ($attribute != $col) {
						if (false == ($attribute = ldap_next_attribute($this->_db, $entry, $ptr)))
							continue 2;
					}
				}

				if (false == ($values = ldap_get_values($this->_db, $entry, $attribute)))
					throw new DatabaseQueryException(ldap_error($this->_db), $filter);

				if (count($values) <= 2)
					$rows[] = reset($values);
				else {
					unset($values['count']);
					$rows[] = $values;
				}
			}
		} while (false != ($entry = ldap_next_entry($this->_db, $entry)));
		
		return $rows;
	}

	function getAssoc($filter, $offset = 0, $limit = null, $basedn = null, $type = synd_query_ldap::SELECT) {
		if (!is_resource($this->_stmt = $this->_statement($filter, $offset, $limit, $basedn, $type)))
			throw new DatabaseQueryException(ldap_error($this->_db), $filter);
		if (false == ($entry = ldap_first_entry($this->_db, $this->_stmt)))
			return array();
	
		for ($i=0; $i<$offset; $i++) {
			if (false == ($entry = ldap_next_entry($this->_db, $entry)))
				return array();
		}
		
		$rows = array();
		do {
			$ptr = null;
			if (false == ($attribute = ldap_first_attribute($this->_db, $entry, $ptr)))
				continue;
			if (false == ($keys = ldap_get_values($this->_db, $entry, $attribute)))
				throw new DatabaseQueryException(ldap_error($this->_db), $filter);

			if (false == ($attribute = ldap_next_attribute($this->_db, $entry, $ptr)))
				continue;
			if (false == ($values = ldap_get_values($this->_db, $entry, $attribute)))
				throw new DatabaseQueryException(ldap_error($this->_db), $filter);
			
			if (count($values) <= 2)
				$rows[reset($keys)] = reset($values);
			else {
				unset($values['count']);
				$rows[reset($keys)] = $values;
			}
		} while (false != ($entry = ldap_next_entry($this->_db, $entry)));
		
		return $rows;
	}
	
	function getRow($filter, $basedn = null, $type = synd_query_ldap::SELECT) {
		return SyndLib::isError($rows = $this->getAll($filter, 0, 1, $basedn, $type)) ? $rows : reset($rows);
	}

	function getOne($filter, $basedn = null, $type = synd_query_ldap::SELECT) {
		return SyndLib::isError($rows = $this->getCol($filter, 0, 0, 1, $basedn, $type)) ? $rows : reset($rows);
	}
	
	function nextId($name) {
		return 0;
	}
	
	function affectedRows() {
		if (is_resource($this->_stmt))
			return ldap_count_entries($this->_stmt);
		return 0;
	}

	function isAlive() {
		return true;
	}

	function getTables()			{return array();}
	function getSequences()			{return array();}
	function getTableKeys($table)	{return array();}

	function getTableStructure($table) {
		require_once 'core/db/TableMetadata.class.inc';
		return new TableMetadata($table);
	}
	
	function quote($value) {
		if (is_string($value)) {
			return str_replace(
				array(
					'*', 
					'(', 
					')',
					'\\',
					"\0",
					), 
				array(
					'\\2a"', 
					'\\28', 
					'\\29',
					'\\5c', 
					'\\00', 
					), $value);
		}
		return $value;
	}
	
	function unixtodate($expr)	{return '';}
	function date($expr)		{return '';}
}

/**
 * LDAP result set
 */
class synd_result_ldap extends AbstractResultSet {
	/**
	 * @access	protected
	 */
	var $_db = null;

	/**
	 * @access	protected
	 */
	var $_stmt = null;
	
	/**
	 * @access	protected
	 */
	var $_entry = null;

	function __construct($db, $stmt, $offset = 0) {
		$this->_db = $db;
		$this->_stmt = $stmt;
		$this->_entry = ldap_first_entry($this->_db->_db, $this->_stmt);
		
		for ($i=0; $i<$offset; $i++) {
			if (false == ($this->_entry = ldap_next_entry($this->_db->_db, $this->_entry)))
				break;
		}
	}
	
	function fetchRow()	{
		$ptr = null;
		if (false == $this->_entry || 
			false == ($attribute = ldap_first_attribute($this->_db->_db, $this->_entry, $ptr)))
			return false;

		$row = array();
		do {
			if (false == ($values = ldap_get_values($this->_db->_db, $this->_entry, $attribute)))
				throw new DatabaseException(ldap_error($this->_db));
			if (count($values) <= 2)
				$row[$attribute] = reset($values);
			else {
				unset($values['count']);
				$row[$attribute] = $values;
			}
		} while (false != ($attribute = ldap_next_attribute($this->_db->_db, $this->_entry, $ptr)));
		
		if (!isset($row['dn']))
			$row['dn'] = ldap_get_dn($this->_db->_db, $this->_entry);

		$this->_entry = ldap_next_entry($this->_db->_db, $this->_entry);
		return $row;
	}
	
	function fetchInto(&$row) {
		return $row = $this->fetchRow();
	}
}

/**
 * Buffered LDAP result set
 */
class synd_rowset_ldap extends AbstractResultSet {
	var $_rows = null;
	
	function __construct($rows) {
		$this->_rows = $rows;
		reset($this->_rows);
	}
	
	function fetchRow() {
		if (false !== ($row = current($this->_rows)))
			next($this->_rows);
		return $row;
	}

	function fetchInto(&$row) {
		return $row = $this->fetchRow();
	}
}
