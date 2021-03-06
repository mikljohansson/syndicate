<?php
require_once 'core/model/type/indexer_task.class.inc';

class synd_type_node_indexer_task extends synd_type_indexer_task {
	/**
	 * Number of seconds between rounds
	 *
	 * The indexer will index all nodes in the queue, sleep the 
	 * specified number of seconds and repeat 3 times. This is 
	 * done so the index doesn't get flushed more than once every
	 * delay seconds in write-heavy environments.
	 *
	 * @access	protected
	 */
	var $_delay = 60;

	/**
	 * @access	private
	 */
	var $_namespace = null;
	
	/**
	 * @access	private
	 */
	var $_key = null;
	
	/**
	 * @access	private
	 */
	var $_keys = null;
	
	/**
	 * @access	private
	 */
	var $_notify = null;
	
	/**
	 * @param	AbstractIndex	Index to run against
	 * @param	string		Namespace of queue
	 * @param	integer		Key of this backend
	 * @param	integer		Union of all backend keys
	 * @param	array		Keys and URNs of other backends to notify indexed by their cluster key
	 */
	function __construct($index, $namespace, $key, $keys, $notify) {
		parent::__construct($index);
		assert('null != $namespace && is_numeric($key) && $key >= 1 && is_numeric($keys) && $keys >= 1');
		$this->_namespace = $namespace;
		$this->_key = $key;
		$this->_keys = $keys;
		$this->_notify = $notify;
	}

	function getBatch($limit) {
		$ids = $this->_fetch($limit);
	
		// Instantiate queue and check for deleted nodes
		$index = $this->getIndex();
		$nodes = SyndNodeLib::getInstances($ids);
		if (count($deleted = array_diff($ids, array_keys($nodes))))
			$index->delDocumentBatch($deleted);
		return array($nodes, $ids);
	}

	function completed($ids) {
		if (count($ids) == 0)
			return;
		
		$search = Module::getInstance('search');
		$database = $search->getIndexDatabase();
		
		$sql = "
			DELETE FROM synd_search_queue
			WHERE 
				namespace = ".$database->quote($this->_namespace)." AND
				node_id IN (".implode(', ', $database->quote($ids)).") AND
				backends + ".$database->quote($this->_key)." >= ".$database->quote($this->_keys);
		$result = $database->query($sql);

		if (SyndLib::isError($result)) {
			trigger_error($result->getMessage(), E_USER_WARNING);
			return;
		}

		if ($database->affectedRows() < count($ids)) {
			$sql = "
				UPDATE synd_search_queue
				SET backends = backends + ".$database->quote($this->_key)."
				WHERE
					namespace = ".$database->quote($this->_namespace)." AND
					node_id IN (".implode(', ', $database->quote($ids)).")";
			$result = $database->query($sql);

			if (SyndLib::isError($result)) {
				trigger_error($result->getMessage(), E_USER_WARNING);
				return;
			}
		}
	}
	
	function skip($i) {
		$ids = $this->_fetch($i);
		$this->completed($ids);
		trigger_error("Indexer: Nodes ".implode(",", $ids)." are causing persistent indexer errors, skipping them", E_USER_WARNING);
	}
	
	function _fetch($limit) {
		$index = $this->getIndex();
		$search = Module::getInstance('search');
		$database = $search->getIndexDatabase();

		$sql = "
			SELECT DISTINCT q.node_id FROM synd_search_queue q
			WHERE 
				q.namespace = ".$database->quote($this->_namespace)." AND
				".$database->bitand('q.backends',$database->quote($this->_key))." = 0";
		$ids = $database->getCol($sql, 0, 0, $limit);

		if (SyndLib::isError($ids)) {
			trigger_error($ids->getMessage(), E_USER_WARNING);
			return array();
		}
		
		return $ids;
	}
	
	function run($handler) {
		if (!empty($this->_notify)) {
			require_once 'core/lib/rpc/RpcTransport.class.inc';
			foreach ($this->_notify as $info) {
				if (null !== ($backend = Activator::getInstance($info[1])) &&
					false !== $backend->runIndexer($this->_namespace, $info[0], $this->_keys))
					print "Notified backend '{$info[1]}' to update its index\r\n";
				else
					print "Failed to notify backend '{$info[1]}' to update its index\r\n";
			}
		}
		
		for ($i = 0, $delay = 5; $i < 3; $i++, $delay = $this->_delay) {
			print "Sleeping $delay seconds before starting round ".($i+1).".\n";
			sleep($delay);

			parent::run($handler);
		}
	}
}
