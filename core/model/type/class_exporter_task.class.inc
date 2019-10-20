<?php
require_once 'core/model/type/task.class.inc';

class synd_type_class_exporter_task extends synd_type_task {
	/**
	 * @access	private
	 */
	var $_offset = 0;
	
	/**
	 * @access	private
	 */
	var $_clsid = null;
	
	/**
	 * @access	private
	 */
	var $_order = null;

	/**
	 * Batch size
	 */
	var $_limit = 750;
	
	/**
	 * Tracks the running failure count used as a modifier for the batch size limit.
	 */
	var $_prevFailures = 0;
	
	/**
	 * Tracks how many batches in a row has been processed successfully
	 */
	var $_prevSuccess = 0;
	
	function __construct($clsid, $order = array()) {
		parent::__construct();
		$this->_clsid = $clsid;
		$this->_order = $order;
	}
	
	function run($handler) {
		parent::run($handler);
		// Increase memory limit if very low to avoid too much out-of-memory problems
		if ($this->_toBytes(ini_get('memory_limit')) < $this->_toBytes('512M'))
			ini_set('memory_limit', '512M');
		
		// Increase time limit to have time to export everything
		set_time_limit(60*5);
		// Retrieves the number of times this exact task instance has already failed, and
		// reset the success count in case of a previous failure on this batch.
		$failures = $handler->getFailureCount();
		if ($failures > 0)
			$this->_prevSuccess = 0;
		// Decrease limit exponentially in case of failure/crashes/out-of-memory
		$limit = $this->_getLimit($failures + $this->_prevFailures);
		print "Exporter: $failures previous failures on this batch, previous failure count is {$this->_prevFailures}, batch size is $limit\n";
		
		// Check if this single node causes crashes
		if ($this->_limit > 1 && $limit == 1 && $failures > 0 && $this->_getLimit($failures + $this->_prevFailures - 1) == 1) {
			print "Exporter: Skipping one item due to crashes on that single object\n";
			$this->skip(1);
			$this->_prevFailures = $failures + $this->_prevFailures - 1;
			$handler->appendTask($this, $this->classId().'.'.$this->objectId());
			return;
		}
		
		list($nodes, $ids) = $this->getBatch($limit);
		if (count($nodes) > 0) {
			print 'Exporter: '.date('Y-m-d H:i:s').' Got '.count($nodes)." nodes, exporting ..\n";
			if ($limit == 1) {
				print 'Exporter: Attempting to export single node '.$ids[0]."\n";
			}
			foreach ($nodes as $node) {
				$dname = '/usr/local/ahs/export/'.date('Y-m', $node->data['TS_CREATE']).'/';
				SyndLib::createDirectory($dname);

				$fname = $dname.$node->objectId().'.json';
				$content = $node->data;
				$content['project'] = $node->getParent()->data;

				$content['events'] = array();
				foreach (SyndLib::sort($node->getEventLogger()->getEvents(), 'getTimestamp') as $event) {
					if ($event instanceof synd_type_onsetparent) {
						$content['events'][] = array(
							'event' => 'moved-to', 'timestamp' => $event->getTimestamp(), 
							'project' => $event->getProject()->data);
					}

					if ($event instanceof synd_type_onassign) {
						$content['events'][] = array(
							'event' => 'assigned-to', 'timestamp' => $event->getTimestamp(), 
							'assignee' => $event->getAssigned()->data);
					}
				}

				file_put_contents($fname, json_encode($content, JSON_PRETTY_PRINT));

			}
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
			print "Exporter: Rescheduling task after processing ".count($ids)." nodes.\n";
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

	function _fetch($limit) {
		$storage = SyndNodeLib::getDefaultStorage($this->_clsid);
		$storage->flush();

		$persistent = $storage->getPersistentStorage();
		$database = $persistent->getDatabase();
		$query = call_user_func(array(SyndNodeLib::loadClass($this->_clsid), 'getEntityQuery'), $storage);
		$query->column($query->getPrimaryKey());
		$query->where('ts_create >= '.strtotime('-7 days'));

		$project = SyndNodeLib::getInstance('project.48112');
		$pids = $project->getProjectBranchIds();
		$query->in('parent_node_id', $pids); 

		foreach ($this->_order as $value)
			$query->order($value[0], $value[1]);

		$result = $database->execute($query, $this->_offset, $limit);
		$ids = array();

		while (false != ($row = $result->fetchRow())) {
			if (null != ($id = $query->getIdentifier($row)))
				$ids[] = $id;
			$this->_offset++;
		}

		return $ids;
	}
		
	function getBatch($limit) {
		$ids = $this->_fetch($limit);

		$storage = SyndNodeLib::getDefaultStorage($this->_clsid);
		$persistent = $storage->getPersistentStorage();
		$nodes = $persistent->getInstances($ids);
		return array($nodes, $ids);
	}

       	function completed($ids) {}
	
	function skip($i) {
		$ids = $this->_fetch($i);
		trigger_error("Exporter: Nodes ".implode(",", $ids)." are causing persistent exporter errors, skipping them", E_USER_WARNING);
		$this->_offset += $i;
	}
}
