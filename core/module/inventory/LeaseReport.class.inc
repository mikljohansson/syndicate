<?php

/**
 * Abstract lease report
 * @abstract
 * @access		public
 * @package		synd.core.module
 */
class LeaseReport {
	/**
	 * @access	protected
	 */
	var $_storage = null;
	
	/**
	 * @access	protected
	 */
	var $_query = null;

	/** 
	 * @param	DomainStorage		Storage to run against
	 * @param	synd_query_sql	The base SQL query
	 */
	function LeaseReport($storage, $query) {
		$this->_storage = $storage;
		$this->_query = $query;
	}
	
	/**
	 * @return	ISyndCollection
	 */
	function getCollection() {
		$query = clone $this->_query;
		$query->column('NODE_ID');
		return SyndType::factory('query_collection', SyndNodeLib::getDefaultStorage('lease'), $this->_query->toString());
	}
}

/**
 * Simple issue listing
 */
class LeaseListingReport extends LeaseReport {
	/**
	 * @access	protected
	 */
	var $_order = null;

	/** 
	 * @param	DomainStorage		Storage to run against
	 * @param	synd_query_sql	The base SQL query
	 * @param	array			Issue ordering 
	 */
	function __construct($storage, $query, $order = null) {
		parent::__construct($storage, $query);
		$this->_order = $order;
	}
	
	/**
	 * Issues contained in report
	 * @param	integer	Offset in result set
	 * @param	integer	Max issues to return
	 * @return	array
	 */
	function getContents($offset = 0, $limit = null) {
		$l = $this->_query->join('synd_inv_lease','l');
		$query = clone $this->_query;
		$query->column("DISTINCT $l.node_id");
		
		// Add ordering
		foreach (array_values((array)$this->_order) as $key => $column) {
			if (is_string($column) && !empty($column) && !is_numeric($column)) {
				$query->column("$l.$column");
				$query->order($column, !isset($columns[$key+1]) || !empty($columns[$key+1]));
			}
		}

		$persistent = $this->_storage->getPersistentStorage();
		$database = $persistent->getDatabase();
		return $this->_storage->getInstances($database->getCol($query->toString(), 0, $offset, $limit));
	}
	
	/**
	 * Number of issues contained in report
	 * @param	integer	Offset in result set
	 * @param	integer	Max issues to return
	 * @return	integer
	 */
	function getCount() {
		$l = $this->_query->join('synd_inv_lease','l');
		$query = clone $this->_query;
		$query->column("COUNT(DISTINCT $l.node_id)");

		$persistent = $this->_storage->getPersistentStorage();
		$database = $persistent->getDatabase();
		
		return $database->getOne($query->toString());
	}
}
