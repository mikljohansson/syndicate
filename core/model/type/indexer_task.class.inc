<?php
require_once 'core/model/type/task.class.inc';
require_once 'core/index/AbstractIndex.class.inc';

class synd_type_indexer_task extends synd_type_task {
	var $_buffer = null;
	var $_index = null;
	var $_max = 500;
	
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

	function getBatch() {
		return array();
	}
	
	function run($handler) {
		parent::run($handler);
		
		$count = 0;
		$index = $this->getIndex();
		$indexer = $index->getIndexer();
		$resched = false;
		
		while ($count < $this->_max && count($batch = $this->getBatch())) {
			$count += count($batch);
			print date('Y-m-d H:i:s').' Got '.count($batch)." ($count total) nodes, indexing ..\n";

			foreach ($batch as $node) {
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

			// Check memory usage
			$limit = ini_get('memory_limit')*1024*1024;
			$usage = memory_get_usage(true);

			if ($usage >= $limit * 0.5) {
				print "Memory usage (".$usage." bytes) exceeded 50% of memory_limit (".$limit." bytes), rescheduling task\n";
				$resched = true;
				break;
			}
		}

		// Reschedule task if max nodes indexed
		if ($count >= $this->_max || $resched) {
			print "Max nodes ($this->_max) reached, rescheduling task.\n";
			$handler->appendTask($this, $this->classId().'.'.$this->objectId());
			return false;
		}
		
		return true;
	}
}
