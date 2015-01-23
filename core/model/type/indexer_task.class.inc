<?php
require_once 'core/model/type/task.class.inc';
require_once 'core/index/AbstractIndex.class.inc';

class synd_type_indexer_task extends synd_type_task {
	var $_buffer = null;
	var $_index = null;
	var $_limit = 500;
	
	/**
	 * Tracks the running failure count used a modifier for the batch size limit.
	 */
	var $_prevFailures = 0;
	
	/**
	 * Tracks how many batches in a row has been processed successfully
	 */
	var $_prevSuccess = 0;
	
	function __construct($index, $oid = null) {
		parent::__construct($oid);
		$this->_buffer = serialize($index);
	}

	function __sleep() {
		$vars = array_keys(get_object_vars($this));
		unset($vars[array_search('_index', $vars)]);
		return $vars;
	}

	function getIndex() {
		if (null == $this->_index)
			$this->_index = unserialize($this->_buffer);
		return $this->_index;
	}

	function getBatch($limit) {
		return array(array(), array());
	}
	
	function completed($nodes, $ids) {}
	
	function run($handler) {
		parent::run($handler);

		ini_set('memory_limit', '2048M');
		set_time_limit(60*30);

		// Reset the success count in case of a previous failure on this same batch
		if ($handler->getFailureCount() > 0) {
			$this->_prevSuccess = 0;
		}
		
		// Decrease limit exponentially in case of failure/crashes/out-of-memory
		$failures = $handler->getFailureCount() + $this->_prevFailures;
		$limit = max($this->_limit / pow(2, $failures), 1);
		
		$index = $this->getIndex();
		$indexer = $index->getIndexer();
		list($nodes, $ids) = $this->getBatch($limit);

		if (count($nodes) > 0) {
			print date('Y-m-d H:i:s').' Got '.count($nodes)." ($count total) nodes, indexing ..\n";
			foreach ($nodes as $node) {
				$indexer->open($node->id(), $node->getIndexSection(), $node->getSecurityAttributes());

				try {
					$node->acceptIndexer($indexer);
				}
				catch (DatabaseException $e) {
					print date('Y-m-d H:i:s').' '.$e->getMessage()."\n";
					sleep(1);
				}

				$indexer->close();
			}

			$indexer->flush();
		}
		
		$this->completed($nodes, $ids);
		
		// Keep the running failure and success counts
		$this->_prevFailures = max($handler->getFailureCount(), $this->_prevFailures);
		$this->_prevSuccess++;
		
		// Try increasing the batch size again after 10 successesful batches
		if ($this->_prevSuccess >= 10) {
			$this->_prevFailures = max($this->_prevFailures - 1, 0);
			$this->_prevSuccess = 0;
		}

		// Reschedule task if limit number of nodes was returned
		if (count($ids) >= $limit) {
			print "Rescheduling task after processing ".count($ids)." nodes.\n";
			$handler->appendTask($this, $this->classId().'.'.$this->objectId());
		}
	}
}
