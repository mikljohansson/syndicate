<?php
require_once 'core/db/Database.class.inc';

/**
 * Implements an LDAP query
 * @access		public
 * @package		synd.core.db
 */
class synd_query_ldap implements DatabaseQuery {
	const SELECT = 0;
	const SEARCH = 1;
	
	/**
	 * Database to run this query
	 * @var		Database
	 */
	protected $_db = null;

	/**
	 * Query type {synd_query_ldap::SELECT, synd_query_ldap::SEARCH}
	 * @var		int
	 */
	protected $_type = 1;
	
	/**
	 * Columns to be selected
	 * @var		array
	 */
	protected $_columns = array();
	
	/**
	 * Holds the basedn
	 * @var		array
	 */
	protected $_basedn = null;

	/**
	 * Holds the WHERE clauses
	 * @var		array
	 */
	protected $_where = array();

	/**
	 * Holds the order by statement
	 * @var		string
	 */
	protected $_order = null;
	
	/**
	 * IN restricted columns
	 * @var		array
	 */
	protected $_inv = null;
	
	/**
	 * @param	Database	Database to run this query
	 */
	function __construct(Database $db, $type = self::SEARCH) {
		$this->_db = $db;
		$this->_type = $type;
	}
	
	function getIterator($offset = 0, $limit = null) {
		return $this->_db->query($this->toString(), $offset, $limit, $this->_basedn, $this->_type);
	}
	
	function count() {
		switch ($this->_type) {
			case self::SELECT:
				$stmt = ldap_list($this->_db->getHandle(), $this->_basedn, $this->toString());
				break;

			case self::SEARCH:
				$stmt = ldap_search($this->_db->getHandle(), $this->_basedn, $this->toString());
				break;

			default:
				throw new Exception("Unknown query type '$type'");
		}
		
		if (false !== $stmt && false !== ($count = ldap_count_entries($this->_db->getHandle(), $stmt)))
			return $count;
		
		throw new DatabaseException();
	}

	function getCol($col = 0, $offset = 0, $limit = null) {
		return $this->_db->getCol($this->toString(), $col, $offset, $limit, $this->_basedn, $this->_type);
	}
	
	function getOne() {
		return $this->_db->getOne($this->toString(), $this->_basedn, $this->_type);
	}
	
	function toString() {
		$where = $this->_where;
		$result = '(&';
		
		if (!empty($where))
			$result .= implode('', $where);
		
		if (!empty($this->_inv)) {
			foreach ($this->_inv as $in) {
				$result .= $in;
				$result .= ')';
			}
		}
		
		$result .= ')';
		return '(&)' != $result ? $result : '(objectClass=*)';
	}
	
	function column($expression) {
		$this->_columns[] = $expression;
	}
	
	function getColumns() {
		return $this->_columns;
	}
	
	function clearColumns() {
		$this->_columns = array();
	}
	
	function join($table, $prefix = null, $keys = null) {
		$this->_basedn = $table;
	}
	
	function getTables() {
		return array($this->_basedn);
	}
	
	function where($column, $value = null) {
		if (null != $column)
			$this->_where[] = null !== $value ? "($column=".$this->_db->quote($value).")" : $column;
	}
	
	function in($column, $value) {
		if (!empty($value)) {
			if (is_scalar($value)) {
				if (!isset($this->_inv[$column]))
					$this->_inv[$column] = '(|';
				$this->_inv[$column] .= "($column=";
				$this->_inv[$column] .= $this->_db->quote($value);
				$this->_inv[$column] .= ")";
			}
			else {
				foreach ($value as $v)
					$this->in($column, $v);
			}
		}
	}

	function order($column, $ascending = true) {
		$this->_order[$column] = array($column, $ascending);
	}
}
