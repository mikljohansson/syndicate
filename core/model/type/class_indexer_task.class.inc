<?php
require_once 'core/model/type/indexer_task.class.inc';

class synd_type_class_indexer_task extends synd_type_indexer_task {
	/**
	 * @access	private
	 */
	var $_offset = 0;
	
	/**
	 * The number of nodes in each batch (<= 1000)
	 * @access	protected
	 */
	var $_limit  = 100;
	
	/**
	 * @access	private
	 */
	var $_clsid = null;
	
	function __construct($index, $clsid, $order = array()) {
		parent::__construct($index, $clsid);
		$this->_clsid = $clsid;
		$this->_order = $order;
	}
	
	function getBatch() {
		ini_set('memory_limit', '2048M');
		set_time_limit(60*30);

		$storage = SyndNodeLib::getDefaultStorage($this->_clsid);
		$storage->flush();

		$persistent = $storage->getPersistentStorage();
		$database = $persistent->getDatabase();
		$query = call_user_func(array(SyndNodeLib::loadClass($this->_clsid), 'getEntityQuery'), $storage);
		$query->column($query->getPrimaryKey());

		foreach ($this->_order as $value)
			$query->order($value[0], $value[1]);

		$result = $database->execute($query, $this->_offset, $this->_limit);
		$ids = array();

		while (false != ($row = $result->fetchRow())) {
			if (null != ($id = $query->getIdentifier($row)))
				$ids[] = $id;
			$this->_offset++;
		}
		
		return $persistent->getInstances($ids);
	}
}
