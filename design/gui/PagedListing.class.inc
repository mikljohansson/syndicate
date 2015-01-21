<?php

/**
 * Simplifies paged listings
 * @access		public
 * @package		synd.design
 */
class PagedListing {
	var $_instances = null;
	var $_storage = null;
	var $_query = null;
	var $_db = null;

	var $_order = null;
	var $_count = null;
	var $_limit = null;
	var $_id = null;
	var $_req = null;
	
	function PagedListing($storage, $query, $order = null, $limit = 75, $id = null, $request = null) {
		$this->_storage = $storage;
		$this->_query = $query;
		
		$persistent = $storage->getPersistentStorage();
		$this->_db = $persistent->getDatabase();
		
		$this->_order = $order;
		$this->_limit = $limit;
		$this->_id = $id;
		$this->_req = null !== $request ? $request : $_REQUEST;
	}
	
	function getOffset() {
		$id = null != $this->_id ? $this->_id : 'offset';
		return isset($this->_req[$id]) && is_numeric($this->_req[$id]) ? $this->_req[$id] : 0;
	}
	
	function getLimit() {
		return $this->_limit;
	}

	function getCount() {
		if (null === $this->_count) {
			profiler_enter(__METHOD__);
			$query = clone $this->_query;
			$query->clearColumns();
			$query->column('COUNT(*)');
			$this->_count = $this->_db->getOne($query->toString());
			profiler_leave(__METHOD__);
		}
		return $this->_count;
	}
	
	/**
	 * Returns the raw resultset
	 * @return	ResultSet
	 */
	function getResult() {
		$database = $this->_storage->getDatabase();
		$result = $database->query($this->_query->toString());
		return $result;
	}
	
	function getInstances() {
		if (null === $this->_instances) {
			if (empty($this->_order))
				$sql = $this->_query->toString();
			else {
				$nquery = clone $this->_query;
				foreach ($this->_order as $i => $column) {
					if (is_string($column) && !is_numeric($column) && !empty($column)) {
						$nquery->column($column);
						$nquery->order($column, !isset($this->_order[$i+1]) || !empty($this->_order[$i+1]));
					}
				}
				$sql = $nquery->toString();
			}
			
			profiler_enter(__METHOD__);
			$ids = $this->_db->getCol($sql, 0, $this->getOffset(), $this->getLimit(), null, SYND_SQL_CALC_FOUND_ROWS);
			$this->_count = null !== ($count = $this->_db->foundRows()) ? $count : $this->_count;
			$this->_instances = $this->_storage->getInstances($ids);
			profiler_leave(__METHOD__);
		}
		return $this->_instances;
	}
	
	function getParameters() {
		return array(
			'offset' => $this->getOffset(),
			'limit' => $this->getLimit(),
			'count' => $this->getCount(),
			'offset_variable_name' => null != $this->_id ? $this->_id : 'offset');
	}
}
