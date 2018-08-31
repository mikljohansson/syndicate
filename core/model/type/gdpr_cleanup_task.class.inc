<?php
require_once 'core/model/type/task.class.inc';

class synd_type_gdpr_cleanup_task extends synd_type_task {
	var $_limit = 750;
	
	/**
	 * Tracks the running failure count used as a modifier for the batch size limit.
	 */
	var $_prevFailures = 0;
	
	/**
	 * Tracks how many batches in a row has been processed successfully
	 */
	var $_prevSuccess = 0;
	
	function _fetch($limit) {
		global $synd_maindb;

		// List of issues keyword names to clean up
		$keywords = array('GDPR-gallring');

		$sql = "
		        SELECT i.node_id
		        FROM synd_issue i, synd_project p
		        WHERE
		                i.parent_node_id = p.node_id AND
		                i.ts_update < (".strtotime('midnight')." - p.info_cleanup_cutoff_seconds)

		        UNION ALL

		        SELECT i.node_id
		        FROM synd_issue i, synd_issue_keyword ik, synd_keyword k
		        WHERE
		                i.node_id = ik.issue_node_id AND
		                k.node_id = ik.keyword_node_id AND
		                ik.ts_create < ".strtotime('midnight')." AND
		                k.info_head IN (".implode(',', $synd_maindb->quote($keywords)).")";

		$ids = $synd_maindb->getCol($sql, 0, 0, $limit);
		return $ids;
	}

	function getBatch($limit) {
		$ids = $this->_fetch($limit);
		$nodes = SyndNodeLib::getInstances($ids);
		return array($nodes, $ids);
	}
	
	function skip($i) {
		global $synd_maindb;

		$ids = $this->_fetch($i);
		$sql = "
			UPDATE synd_issue i
			SET i.ts_update = ".time()."
			WHERE i.node_id IN (".implode(',',$synd_maindb->quote($ids)).")";
		$synd_maindb->query($sql);

		trigger_error("GDPR cleanup: Nodes ".implode(",", $ids)." are causing persistent errors, skipping them", E_USER_WARNING);
	}
	
	function run($handler) {
		parent::run($handler);

		// Increase memory limit if very low to avoid too much out-of-memory problems
		if ($this->_toBytes(ini_get('memory_limit')) < $this->_toBytes('512M'))
			ini_set('memory_limit', '512M');
		
		// Increase time limit to have time to index everything
		set_time_limit(60*5);

		// Retrieves the number of times this exact task instance has already failed, and
		// reset the success count in case of a previous failure on this batch.
		$failures = $handler->getFailureCount();
		if ($failures > 0)
			$this->_prevSuccess = 0;

		// Decrease limit exponentially in case of failure/crashes/out-of-memory
		$limit = $this->_getLimit($failures + $this->_prevFailures);
		print "GDPR cleanup: $failures previous failures on this batch, previous failure count is {$this->_prevFailures}, batch size is $limit\n";
		
		// Check if this single node causes crashes
		if ($this->_limit > 1 && $limit == 1 && $failures > 0 && $this->_getLimit($failures + $this->_prevFailures - 1) == 1) {
			print "GDPR cleanup: Skipping one item due to crashes on that single object\n";
			$this->skip(1);
			$this->_prevFailures = $failures + $this->_prevFailures - 1;
			$handler->appendTask($this, $this->classId().'.'.$this->objectId());
			return;
		}

		$module = Module::getInstance('issue');
		list($nodes, $ids) = $this->getBatch($limit);

		if (count($nodes) > 0) {
			print 'GDPR cleanup: '.date('Y-m-d H:i:s').' Got '.count($nodes)." nodes, cleaning ..\n";
			if ($limit == 1) {
				print 'GDPR cleanup: Attempting to clean single node '.$ids[0]."\n";
			}

			foreach ($nodes as $node) {
				$module->cleanup($node);

				// A bit of sleep to avoid overloading the system in case of failures
				usleep(10 * 1000);
			}
		}
		
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
			print "GDPR cleanup: Rescheduling task after processing ".count($ids)." nodes (".implode(", ",array_slice($ids,0,10))." ...)\n";
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
