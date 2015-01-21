<?php
require_once 'core/model/type/collection.class.inc';
require_once 'core/db/SyndDBLib.class.inc';

class synd_type_search_collection extends synd_type_collection {
	var $_clsid = null;
	var $_query = null;
	var $_order = null;
	var $_count = null;
	var $_operator = null;
	
	function __construct($clsid, $query, $order = null, $operator = '&&') {
		parent::__construct();
		$this->_clsid = $clsid;
		$this->_query = $query;
		$this->_order = $order;
		$this->_operator = $operator;
	}

	function __sleep() {
		$vars = array_keys(get_object_vars($this));
		unset($vars[array_search('_count', $vars)]);
		return $vars;
	}
	
	function toString() {
		return $this->_query;
	}

	function getCount() {
		if (null === $this->_count && null != ($class = SyndNodeLib::loadClass($this->_clsid))) {
			$storage = SyndNodeLib::getDefaultStorage($this->_clsid);
			$database = $storage->getDatabase();

			$filters = array();
			$query = call_user_func(array($class, 'getEntityQuery'), $storage);
			$query->column('COUNT(*)', 'SYND_SEARCH_COUNT');

			foreach ($query->getTables() as $table) {
				$columns = SyndLib::array_prepend($database->getTableStructure($table)->getColumnNames(), $query->join($table).'.');
				$filters = array_merge((array)SyndDBLib::sqlLikeExpr($this->_query, $columns, '||' == $this->_operator ? 'OR' : 'AND', 2), $filters);
			}

			if (empty($filters)) 
				$this->_count = 0;
			else {
				$query->where('('.implode(' OR ', $filters).')');
				$this->_count = $database->getOne($query->toString($database));
			}
		}
		return $this->_count;
	}

	function getContents($offset = 0, $limit = null) {
		$nodes = array();

		if (null != ($class = SyndNodeLib::loadClass($this->_clsid))) {
			$storage = SyndNodeLib::getDefaultStorage($this->_clsid);
			$database = $storage->getDatabase();

			$query = call_user_func(array($class, 'getEntityQuery'), $storage);
			$filters = array();

			foreach ($query->getTables() as $table) {
				$columns = SyndLib::array_prepend($database->getTableStructure($table)->getColumnNames(), $query->join($table).'.');
				$filters = array_merge((array)SyndDBLib::sqlLikeExpr($this->_query, $columns, '||' == $this->_operator ? 'OR' : 'AND', 2), $filters);
			}

			if (!empty($filters)) {
				$query->where('('.implode(' OR ', $filters).')');
				for ($i=0; $i<count($this->_order); $i++) {
					if (is_string($this->_order[$i]) && '' != $this->_order[$i] && !is_numeric($this->_order[$i]))
						$query->order($this->_order[$i], isset($this->_order[$i+1]) ? (bool)$this->_order[$i+1] : true);
				}

				$rows = $database->getAll($query->toString($database), $offset, $limit);
				assert('!SyndLib::isError($rows)');
				
				foreach ($rows as $row) {
					if (null != ($id = $query->getIdentifier($row)))
						$storage->preload($id);
				}

				foreach ($rows as $row) {
					if (null !== ($node = $storage->getInstance($query->getIdentifier($row))))
						$nodes[$node->id()] = $node;
				}
			}
		}
		
		return $nodes;
	}

	function getFilteredCount($classes) {
		if (in_array(SyndNodeLib::loadClass($this->_clsid), $classes))
			return $this->getCount();
		return 0;
	}
	
	function getFilteredContents($classes) {
		if (in_array(SyndNodeLib::loadClass($this->_clsid), $classes))
			return $this->getContents();
		return array();
	}
	
	/**
	 * @access	protected
	 */
	function isInheritedFrom($class, $class2) {
		do {
			if ($class == $class2)
				return true;
		} while (null != ($class = get_parent_class($class)) && 'syndnode' != $class);
		return false;
	}
}
