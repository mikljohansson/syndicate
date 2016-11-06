<?php
require_once 'core/model/type/task.class.inc';
require_once 'core/index/AbstractIndex.class.inc';

class synd_type_indexer_task extends synd_type_task {
	var $_buffer = null;
	var $_index = null;
	var $_limit = 750;
	
	/**
	 * Tracks the running failure count used as a modifier for the batch size limit.
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
	
	function completed($ids) {}
	
	function skip($i) {}
	
	function run($handler) {
		parent::run($handler);

		// Increase memory limit if very low to avoid too much out-of-memory problems
		if ($this->_toBytes(ini_get('memory_limit')) < $this->_toBytes('512M'))
			ini_set('memory_limit', '512M');
		
		// Increase time limit to have time to index everything
		set_time_limit(60*30);

		// Reset the success count in case of a previous failure on this same batch
		$failures = $handler->getFailureCount();
		if ($failures > 0)
			$this->_prevSuccess = 0;

		// Decrease limit exponentially in case of failure/crashes/out-of-memory
		$limit = $this->_getLimit($failures + $this->_prevFailures);
		print "Indexer: $failures previous failures on this batch, previous failure count is {$this->_prevFailures}, batch size is $limit\n";
		
		// Check if this single node causes crashes
		if ($this->_limit > 1 && $limit == 1 && $this->_getLimit($failures + $this->_prevFailures - 1) == 1) {
			print "Indexer: Skipping one item due to crashes on that single object\n";
			$this->skip(1);
			$this->_prevFailures = $failures + $this->_prevFailures - 1;
			$handler->appendTask($this, $this->classId().'.'.$this->objectId());
			return;
		}
		
		$index = $this->getIndex();
		$indexer = $index->getIndexer();
		list($nodes, $ids) = $this->getBatch($limit);

		if (count($nodes) > 0) {
			print 'Indexer: '.date('Y-m-d H:i:s').' Got '.count($nodes)." nodes, indexing ..\n";
			foreach ($nodes as $node) {
				$indexer->open($node->id(), $node->getIndexSection(), $node->getSecurityAttributes());

				try {
					$node->acceptIndexer($indexer);
				}
				catch (DatabaseException $e) {
					print 'Indexer: '.date('Y-m-d H:i:s').' '.$e->getMessage()."\n";
					sleep(1);
				}

				$indexer->close();
			}

			$indexer->flush();
		}
		
		$this->completed($ids);
		
		// Keep the running failure and success counts
		$this->_prevFailures += $failures;
		$this->_prevSuccess++;
		
		// Try increasing the batch size again after some successesful batches
		if ($this->_prevSuccess >= 5) {
			$this->_prevFailures = max($this->_prevFailures - 1, 0);
			$this->_prevSuccess = 0;
		}

		// Reschedule task if limit number of nodes was returned
		if (count($ids) >= $limit) {
			print "Indexer: Rescheduling task after processing ".count($ids)." nodes.\n";
			$handler->appendTask($this, $this->classId().'.'.$this->objectId());
		}
	}

	function _getLimit($failures) {
		return (int)max($this->_limit / pow(2, $failures), 1);
	}
	
	function _toBytes($val) {
		if ($val == null)
			return 0;
		
		$val = trim($val);
		$last = strtolower($val[strlen($val)-1]);
		switch($last) {
			// The 'G' modifier is available since PHP 5.1.0
			case 'g':
				$val *= 1024;
			case 'm':
				$val *= 1024;
			case 'k':
				$val *= 1024;
		}

		return $val;
	}
}
