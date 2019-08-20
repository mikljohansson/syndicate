<?php
require_once 'core/db/Database.class.inc';

/**
 * Implements an SQL query
 * @access		public
 * @package		synd.core.db
 */
class synd_query_sql implements DatabaseQuery {
	/**
	 * Database to run this query
	 * @var		Database
	 */
	protected $_db = null;

	/**
	 * Prefix to prepend to all aliases
	 * @var		string
	 */
	protected $_prefix = null;
	
	/**
	 * The current table allocation context
	 * @var		array
	 */
	protected $_context = null;

	/**
	 * The table allocation context stack
	 * @var		array
	 */
	protected $_stack = null;

	/**
	 * Columns to be selected
	 * @var		array
	 */
	protected $_columns = array();
	
	/**
	 * Holds the joined tables
	 * @var		array
	 */
	protected $_join = array();
	
	/**
	 * Holds to FROM table clauses 
	 * @var		array
	 */
	protected $_from = array();

	/**
	 * Holds to involved tables indexed by alias
	 * @var		array
	 */
	protected $_tables = array();

	/**
	 * Holds the WHERE clauses
	 * @var		array
	 */
	protected $_where = array();

	/**
	 * Holds the GROUP BY columns
	 * @var		array
	 */
	protected $_group = null;
	
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
	protected $_inc = null;
	
	/**
	 * @param	Database	Database to run this query
	 * @param	string		Prefix to prepend to all aliases
	 */
	function __construct(Database $db, $prefix = null) {
		$this->_db = $db;
		$this->_prefix = $prefix;
	}
	
	function getIterator($offset = 0, $limit = null) {
		return $this->_db->query($this->toString(), $offset, $limit);
	}
	
	function count() {
		$query = clone $this;
		$query->_columns = array();
		$query->_order = null;
		$query->column('COUNT(1)');
		return $this->_db->getOne($query->toString());
	}
	
	function getCol($col = 0, $offset = 0, $limit = null) {
		return $this->_db->getCol($this->toString(), $col, $offset, $limit);
	}
	
	function getOne() {
		return $this->_db->getOne($this->toString());
	}
	
	function toString() {
		$columns = null;
		foreach ($this->getColumns() as $alias => $expression) {
			if (null != $columns)
				$columns .= ', ';
			if ($alias != $expression)
				$columns .= "$expression $alias";
			else
				$columns .= $expression;
		}
		
		$sql = "
			SELECT $columns
			FROM ".$this->getFrom();
		if (null != ($where = $this->getWhere())) {
			$sql .= "
			WHERE $where";
		}
		
		if (!empty($this->_group)) {
			$sql .= "
			GROUP BY ".implode(', ', $this->_group);
		}

		if (!empty($this->_order)) {
			$sql .= "
			ORDER BY ".implode(', ', $this->_order);
		}
		
		return $sql;
	}
	
	function column($expression, $alias = null, $operator = null) {
		if (null == $alias)
			$alias = $expression;
		if (isset($this->_columns[$alias]) && null != $operator)
			$this->_columns[$alias] = "({$this->_columns[$alias]}) $operator ($expression)";
		else
			$this->_columns[$alias] = $expression;
	}
	
	function getColumns() {
		if (!empty($this->_columns))
			return $this->_columns;
		
		$result = array();
		$seen = array();
		
		foreach ($this->getTables() as $alias => $table) {
			foreach ($this->_db->getTableStructure($table)->getColumnNames() as $column) {
				if (!in_array($column, $seen)) {
					$result[$alias.'.'.$column] = $alias.'.'.$column;
					$seen[] = $column;
				}
			}
		}
		
		return $result;
	}
	
	function clearColumns() {
		$this->_columns = array();
	}
	
	function join($table, $prefix = 't', $keys = null) {
		if (!isset($this->_context[$table]))
			$this->_context[$table] = 0;
		
		$alias = $this->_prefix.$prefix.$this->_context[$table];
		if (!isset($this->_from[$alias]))
			$this->_from[$alias] = "$table $alias";
		$this->_tables[$alias] = $table;
		
		if (!empty($keys)) {
			foreach ($keys as $key) {
				if (!isset($this->_join[$key][$table]))
					$this->_join[$key][$table] = array();
				if (false === array_search($alias, $this->_join[$key][$table])) {
					$context = SyndLib::array_kintersect($this->_context, $this->_join[$key]);
					$offset = empty($context) ? 0 : max($context);
					while (isset($this->_join[$key][$table][$offset]) && $offset)
						$offset--;
					assert('!isset($this->_join[$key][$table][$offset])');
					$this->_join[$key][$table][$offset] = $alias;
				}
			}
		}

		return $alias;
	}
	
	/**
	 * Left joins a table 
	 * @param	string	The table alias to join on 
	 * @param	string	The table and alias to be joined
	 * @param	string	The condition to join on
	 */
	function left($alias, $table, $on) {
		assert('isset($this->_from[$alias])');
		if (!isset($this->_context[$table]))
			$this->_context[$table] = 0;
		
		$lt = $this->_prefix.'l'.$this->_context[$table];
		$this->_from[$alias] .= " LEFT JOIN $table $lt ON ($on)";
		$this->_tables[$lt] = $table;
		
		return $lt;
	}
	
	/**
	 * Allocates and joins a table instance from the current context.
	 * @param	string	Table to create instance of
	 * @param	string	Desired alias prefix, a number will be appended to this.
	 * @param	array	Columns to join on
	 * @return	string	Table instance alias (t1,t2,...)
	 */
	function allocate($table, $prefix, $keys = array()) {
		if (!isset($this->_context[$table]))
			$this->_context[$table] = -1;
		$this->_context[$table]++;
		return $this->join($table, $prefix, $keys);
	}
	
	/**
	 * Returns the FROM clause
	 *
	 * Returns the cumulated tables as a FROM expression like:
	 * 'synd_search_document d1, synd_search_termindex t1, 
	 *  synd_search_termindex t2, synd_search_termindex t3'
	 *
	 * @return	string
	 */
	function getFrom() {
		return implode(', ', $this->_from);
	}
	
	function getTables() {
		return $this->_tables;
	}
	
	function where($column, $value = SYND_SQL_MISSING_PARAM) {
		if (null != $column) {
			// column is a complete predicate
			if ($value === SYND_SQL_MISSING_PARAM)
				$this->_where[] = $column;		
			else if ($value !== null)
				$this->_where[] = "$column = ".$this->_db->quote($value);
			else
				$this->_where[] = "$column IS NULL";
		}
	}
	
	/**
	 * Returns the WHERE clause
	 * @return	string
	 */
	function getWhere() {
		$where = $this->_where;
		$result = '';
		
		foreach ($this->_join as $key => $aliases) {
			// Choose a driving table
			$primary = array_shift($aliases);
			
			for ($i=0, $break=false; !$break; $i++) {
				// Choose a current driving alias
				if (!empty($primary))
					$driving = array_shift($primary);
				$break = true;
				
				// Join alias from each table with driving alias
				foreach (array_keys($aliases) as $table) {
					if (isset($aliases[$table][$i]))
						$where[] = "$driving.$key = {$aliases[$table][$i]}.$key";
					if (max(array_keys($aliases[$table])) > $i)
						$break = false;
				}
			}
			
		}
		
		if (!empty($where)) {
			if (!empty($result))
				$result .= ' AND ';
			$result .= implode(' AND ', $where);
		}
		
		if (!empty($this->_inv)) {
			foreach ($this->_inv as $in) {
				if (!empty($result))
					$result .= ' AND ';
				$result .= $in;
				$result .= ')';
			}
		}
		
		return $result;
	}
	
	function in($column, $value) {
		if (null != $value) {
			if (is_scalar($value)) {
				if (!isset($this->_inc[$column])) {
					$this->_inc[$column] = 0;
					$this->_inv[$column] = '';
				}

				if (($this->_inc[$column] % 1000)) 
					$this->_inv[$column] .= ',';
				else {
					if ($this->_inc[$column])
						$this->_inv[$column] .= ') OR ';
					$this->_inv[$column] .= "$column IN (";
				}

				$this->_inv[$column] .= $this->_db->quote($value);
				$this->_inc[$column]++;
			}
			else {
				foreach ($value as $v)
					$this->in($column, $v);
			}
		}
	}

	/**
	 * Adds a new column to group by
	 * @param	string	Column like 'd1.docid'
	 */
	function groupBy($column) {
		if (null === $this->_group || false === array_search($column, $this->_group))
			$this->_group[] = $column;
	}
	
	function order($column, $ascending = true) {
		$this->_order[$column] = $column;
		if (!$ascending)
			$this->_order[$column] .= ' DESC';
	}
	
	/**
	 * Returns the ORDER BY columns
	 * @return	array
	 */
	function getOrderBy() {
		return (array)$this->_order;
	}

	/**
	 * Store the current table allocation context on the stack
	 */
	function pushContext() {
		$this->_stack[] = $this->_context;
	}
	
	/**
	 * Restore a table allocation context from the stack
	 */
	function popContext() {
		$this->_context = array_pop($this->_stack);
	}
}
