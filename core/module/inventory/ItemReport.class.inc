<?php

/**
 * Abstract lease report
 * @abstract
 * @access		public
 * @package		synd.core.module
 */
class ItemReport {
	/**
	 * @access	protected
	 */
	var $_storage = null;

	/**
	 * @access	protected
	 */
	var $_query = null;

	/**
	 * @access	protected
	 */
	var $_db = null;
	
	/** 
	 * @param	DomainStorage		Storage to run against
	 * @param	synd_query_sql	The base SQL query
	 */
	function ItemReport($storage, $query) {
		$this->_storage = $storage;
		$this->_query = $query;
		$persistent = $storage->getPersistentStorage();
		$this->_db = $persistent->getDatabase();
	}
}

/**
 * Simple issue listing
 */
class ItemListingReport extends ItemReport {
	/**
	 * @access	protected
	 */
	var $_order = null;

	/** 
	 * @param	DomainStorage		Storage to run against
	 * @param	synd_query_sql	The base SQL query
	 * @param	array			Issue ordering 
	 */
	function ItemListingReport($storage, $query, $order = null) {
		parent::ItemReport($storage, $query);
		$this->_order = $order;
	}
	
	/**
	 * Issues contained in report
	 * @param	integer	Offset in result set
	 * @param	integer	Max issues to return
	 * @return	array
	 */
	function getContents($offset = 0, $limit = null) {
		$i = $this->_query->join('synd_inv_item','i');
		$query = clone $this->_query;
		$query->column("DISTINCT $i.node_id");
		
		// Add ordering
		foreach (array_values((array)$this->_order) as $key => $column) {
			if (is_string($column) && !empty($column) && !is_numeric($column)) {
				$query->column("$i.$column");
				$query->order($column, !isset($columns[$key+1]) || !empty($columns[$key+1]));
			}
		}

		return $this->_storage->getInstances($this->_db->getCol($query->toString(), 0, $offset, $limit));
	}
	
	/**
	 * Number of issues contained in report
	 * @param	integer	Offset in result set
	 * @param	integer	Max issues to return
	 * @return	integer
	 */
	function getCount() {
		$i = $this->_query->join('synd_inv_item','i');
		$query = clone $this->_query;
		$query->column("COUNT(DISTINCT $i.node_id)");
		return $this->_db->getOne($query->toString());
	}
}

/**
 * Item usage report
 */
class UsageTrendsReport extends ItemReport {
	/**
	 * @access	protected
	 */
	var $_ts = null;
	
	/**
	 * @access	protected
	 */
	var $_grouping = null;
	
	function UsageTrendsReport($storage, $query, $ts, $grouping) {
		parent::ItemReport($storage, $query);
		$this->_ts = $ts;
		$this->_grouping = $grouping;
	}
	
	/**
	 * Returns the number of available items at a given time indexed by date
	 * @param	bool	Include items on currently expired leases
	 * @return	array
	 */
	function getAvailableItems($optimistic = true) {
		$query = clone $this->_query;
		$i = $query->join('synd_instance', 'i');
		
		// Left join on the currently active leasings
		$sql = "(
			SELECT u.child_node_id, u.ts_create, l.ts_expire
			FROM synd_inv_used u, synd_inv_lease l
			WHERE u.parent_node_id = l.node_id AND u.ts_expire IS NULL) d0";
		$query->left($i, $sql, "d0.child_node_id = $i.node_id");

		$query->column($this->_grouping->getGroupingKey($this, $query), 'PK');
		$query->column("COUNT(DISTINCT $i.node_id)", 'CNT');
		

		if ($optimistic)
			$query->where('(d0.child_node_id IS NULL OR d0.ts_expire < '.$this->_db->quote($this->_ts).')');
		else {
			$query->where('(d0.child_node_id IS NULL OR 
				d0.ts_expire >= '.$this->_db->quote(time()).' AND 
				d0.ts_expire <= '.$this->_db->quote($this->_ts).')');
		}
			
		$query->groupBy($this->_grouping->getGroupingKey($this, $query));
		
		$sql = $query->toString();
		$rows = $this->_db->getAssoc($sql);
		
		return $rows;
	}
}

/**
 * Item usage report grouping strategy
 * @access		public
 * @package		synd.core.module
 */
class UsageTrendsGrouping {
	function getGroupingKey($report, $query) {
		return null;
	}
}

/**
 * Groups by make, model columns
 */
class UsageTrendsModelGrouping extends UsageTrendsGrouping {
	function getGroupingKey($report, $query) {
		$ii = $query->join('synd_inv_item', 'ii');
		return "CONCAT($ii.info_make, CONCAT(' ', $ii.info_model))";
	}
}
