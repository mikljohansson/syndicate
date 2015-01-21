<?php

/**
 * Abstract issue report
 * @abstract
 * @access		public
 * @package		synd.core.module
 */
class IssueReport {
	/**
	 * @access	protected
	 */
	var $_storage = null;
	
	/**
	 * @access	protected
	 */
	var $_query = null;

	/**
	 * @access	private
	 */
	var $_from = null;

	/**
	 * @access	private
	 */
	var $_to = null;

	/**
	 * @access	protected
	 */
	var $_sensitive = null;

	/** 
	 * @param	DomainStorage		Storage to run against
	 * @param	synd_query_sql	The base SQL query
	 */
	function IssueReport($storage, $query) {
		$this->_storage = $storage;
		$this->_query = $query;
	}
	
	/**
	 * Set report sensitivity with regards to personel
	 * @return	boolean
	 */
	function setSensitivity($sensitive) {
		$this->_sensitive = $sensitive;
	}

	/**
	 * Is report sensitive
	 * @return	bool
	 */
	function isSensitive() {
		return (bool)$this->_sensitive;
	}
}

/**
 * Simple issue listing
 */
class IssueListingReport extends IssueReport {
	/**
	 * @access	protected
	 */
	var $_order = null;

	/** 
	 * @param	DomainStorage		Storage to run against
	 * @param	synd_query_sql	The base SQL query
	 * @param	array			Issue ordering 
	 */
	function IssueListingReport($storage, $query, $order = null) {
		parent::IssueReport($storage, $query);
		$this->_order = $order;
	}
	
	/**
	 * Issues contained in report
	 * @param	integer	Offset in result set
	 * @param	integer	Max issues to return
	 * @return	array
	 */
	function getContents($offset = 0, $limit = null) {
		$i = $this->_query->join('synd_issue','i');
		$query = clone $this->_query;
		$query->column("DISTINCT $i.node_id");
		
		// Add issue ordering
		foreach ($columns = array_values((array)$this->_order) as $key => $column) {
			if (is_string($column) && !empty($column) && !is_numeric($column)) {
				$query->column("$i.$column");
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
		$i = $this->_query->join('synd_issue','i');
		$query = clone $this->_query;
		$query->column("COUNT(DISTINCT $i.node_id)");

		$persistent = $this->_storage->getPersistentStorage();
		$database = $persistent->getDatabase();
		return $database->getOne($query->toString());
	}
}

/**
 * Service level report
 */
class ServiceLevelReport extends IssueReport {
	/**
	 * @access	protected
	 */
	var $_db = null;

	/**
	 * @access	protected
	 */
	var $_taskQuery = null;

	/**
	 * GROUP BY strategy
	 * @access	protected
	 */
	var $_grouping = null;
	
	/**
	 * @param	DomainStorage				Storage to run against
	 * @param	synd_query_sql			The base SQL query
	 * @param	ServiceLevelGrouping	The grouping scheme to use
	 */
	function ServiceLevelReport($storage, $query, $grouping) {
		parent::IssueReport($storage, $query);

		$persistent = $storage->getPersistentStorage();
		$this->_db = $persistent->getDatabase();
		
		$this->_taskQuery = clone $this->_query;
		$this->_grouping = $grouping;

		$this->_grouping->initialize($this->_query);
		$this->_grouping->initialize($this->_taskQuery);

		$ti = $this->_taskQuery->join('synd_issue','i');
		$tt = $this->_taskQuery->join('synd_issue_task','t');
		$this->_taskQuery->where("$ti.node_id = $tt.parent_node_id");
	}
	
	/**
	 * @access	protected
	 */
	function __clone() {
		$this->_query = clone $this->_query;
		$this->_taskQuery = clone $this->_taskQuery;
	}

	/**
	 * Checks if report is grouped by node instances
	 * @return	boolean
	 */
	function hasInstanceGroups() {
		return $this->_grouping->hasInstanceGroups();
	}

	/**
	 * Checks if report is sensitive with regards to personel
	 * @return	boolean
	 */
	function isSensitive() {
		return null !== $this->_sensitive ? $this->_sensitive : $this->_grouping->isSensitive();
	}

	/**
	 * Number of issues received 
	 * @return	array
	 */
	function getReceivedIssues() {
		$i = $this->_query->join('synd_issue','i');
		$query = clone $this->_query;

		$query->column($this->_grouping->getReceivedKey($this, $query), 'PK');
		$query->column("COUNT(DISTINCT $i.node_id)", 'ISSUES');
		$query->groupBy($this->_grouping->getReceivedKey($this, $query));
		
		$sql = $query->toString();
		$rows = $this->_db->getAssoc($sql);
		return $this->_grouping->filter($rows);
	}
	
	/**
	 * Number of issues received and sent to 2nd line support
	 * @return	array
	 */
	function getReceivedSecondLine() {
		$i = $this->_query->join('synd_issue','i');
		$query = clone $this->_query;

		$ic = $query->join('synd_issue', 'ic');
		$query->where("$i.node_id = $ic.issue_node_id");
		$query->where("$i.parent_node_id != $ic.parent_node_id");

		// Performance optimization
		$query->where("$ic.issue_node_id IS NOT NULL");
		
		$query->column($this->_grouping->getReceivedKey($this, $query), 'PK');
		$query->column("COUNT(DISTINCT $i.node_id)", 'ISSUES');
		$query->groupBy($this->_grouping->getReceivedKey($this, $query));
		
		$sql = $query->toString();
		$rows = $this->_db->getAssoc($sql);
		return $this->_grouping->filter($rows);
	}
	
	/**
	 * Number of issues closed
	 * @param	integer	If specified return the number of issues closed with X sends of creation
	 * @return	array
	 */
	function getClosedIssues($interval = null) {
		$i = $this->_query->join('synd_issue','i');
		$query = clone $this->_query;

		$query->where("$i.info_status >= ".synd_node_issue::CLOSED." AND $i.info_status < ".synd_node_issue::MAX_STATUS_VALUE);
		if (null !== $interval)
			$query->where("$i.ts_create >= $i.ts_resolve - ".$this->_db->quote($interval));
		
		$query->column($this->_grouping->getClosedKey($this, $query), 'PK');
		$query->column("COUNT(DISTINCT $i.node_id)", 'ISSUES');
		$query->groupBy($this->_grouping->getClosedKey($this, $query));
		
		$sql = $query->toString();
		$rows = $this->_db->getAssoc($sql);
		return $this->_grouping->filter($rows);
	}
	
	/**
	 * Number of pending issues 
	 * @return	array
	 */
	function getPendingIssues() {
		$i = $this->_query->join('synd_issue','i');
		$query = clone $this->_query;

		$query->where("$i.info_status >= 0 AND $i.info_status < ".synd_node_issue::CLOSED);
		
		$query->column($this->_grouping->getResolveByKey($this, $query), 'PK');
		$query->column("COUNT(DISTINCT $i.node_id)", 'ISSUES');
		$query->groupBy($this->_grouping->getResolveByKey($this, $query));
		
		$sql = $query->toString();
		$rows = $this->_db->getAssoc($sql);
		return $this->_grouping->filter($rows);
	}

	/**
	 * Number of issues waiting for 2nd line support
	 * @return	array
	 */
	function getPendingSecondLine() {
		$i = $this->_query->join('synd_issue','i');
		$query = clone $this->_query;

		$ic = $query->join('synd_issue', 'ic');
		$query->where("$i.node_id = $ic.issue_node_id");
		$query->where("$i.parent_node_id != $ic.parent_node_id");
		$query->where("$ic.info_status >= 0 AND $ic.info_status < ".synd_node_issue::CLOSED);

		// Performance optimization
		$query->where("$ic.issue_node_id IS NOT NULL");
		
		$query->column($this->_grouping->getResolveByKey($this, $query), 'PK');
		$query->column("COUNT(DISTINCT $i.node_id)", 'ISSUES');
		$query->groupBy($this->_grouping->getResolveByKey($this, $query));
		
		$sql = $query->toString();
		$rows = $this->_db->getAssoc($sql);
		return $this->_grouping->filter($rows);
	}

	/**
	 * Total time logged
	 *
	 * Returns arrays containing the logged time and a count of 
	 * how many issues have time logged
	 *
	 * @return	array
	 */
	function getLoggedTime() {
		$tt = $this->_taskQuery->join('synd_issue_task','t');
		$query = clone $this->_taskQuery;

		$query->column($this->_grouping->getReceivedTaskKey($this, $query), 'PK');
		$query->column("COUNT(DISTINCT $tt.parent_node_id)", 'ISSUES');
		$query->column("SUM($tt.info_duration)*60", 'TIME_LOGGED');
		$query->groupBy($this->_grouping->getReceivedTaskKey($this, $query));
		
		$sql = $query->toString();
		$rows = $this->_db->getAll($sql);
		
		$result = array();
		foreach ($rows as $row)
			$result[$row['PK']] = $row;
			
		return $this->_grouping->filter($result);
	}

	/**
	 * Total estimated time
	 *
	 * Returns arrays containing the estimated time and a count of 
	 * how many issues have been estimated. Also how much time has
	 * been logged on those estimated issues.
	 *
	 * @return	array
	 */
	function getEstimatedTime() {
		$i = $this->_query->join('synd_issue','i');
		$estimate = clone $this->_query;
		$logged = clone $this->_query;

		$estimate->where("$i.info_estimate IS NOT NULL");
		$estimate->column($this->_grouping->getResolveByKey($this, $estimate), 'PK');
		$estimate->column("COUNT(DISTINCT $i.node_id)", 'ISSUES');
		$estimate->column("SUM($i.info_estimate)*60", 'TIME_ESTIMATE');
		$estimate->groupBy($this->_grouping->getResolveByKey($this, $estimate));
		
		$t = $logged->join('synd_issue_task', 't');
		$logged->where("$i.node_id = $t.parent_node_id");
		$logged->where("$i.info_estimate IS NOT NULL");
		$logged->column($this->_grouping->getResolveByKey($this, $logged), 'PK');
		$logged->column("SUM($t.info_duration)*60", 'TIME_LOGGED');
		$logged->groupBy($this->_grouping->getResolveByKey($this, $logged));

		$sql = $estimate->toString();
		$rows = $this->_db->getAll($sql);
		
		$sql2 = $logged->toString();
		$time = $this->_db->getAssoc($sql2);

		$result = array();
		foreach ($rows as $row) {
			$result[$row['PK']] = $row;
			$result[$row['PK']]['TIME_LOGGED'] = isset($time[$row['PK']]) ? $time[$row['PK']] : null;
		}
			
		return $this->_grouping->filter($result);
	}
	
	/**
	 * Service level performance statistics
	 *
	 * Time to solution - The average time from issues are create until closed
	 * Time to active - The average time from issues are create until activated (typically first note or email sent to client)
	 * Accuracy - The accuracy of the resolve by date (negative values indicate overdue issues)
	 *
	 * @return	array
	 */
	function getPerformanceStatistics() {
		$i = $this->_query->join('synd_issue','i');
		$query = clone $this->_query;

		$query->where("$i.info_status >= ".synd_node_issue::CLOSED." AND $i.info_status < ".synd_node_issue::MAX_STATUS_VALUE);
		$query->where("$i.ts_resolve < $i.ts_create + 3600*24*30");
		
		$query->column($this->_grouping->getClosedKey($this, $query), 'PK');
		$query->column("ROUND(AVG($i.ts_resolve - $i.ts_create))", 'TIME_TO_SOLUTION');
		$query->column("ROUND(AVG($i.ts_start - $i.ts_create))", 'TIME_TO_ACTIVE');
		$query->column("ROUND(AVG($i.ts_resolve_by - $i.ts_resolve))", 'ACCURACY');
		$query->groupBy($this->_grouping->getClosedKey($this, $query));
		
		$sql = $query->toString();
		$rows = $this->_db->getAll($sql);

		$result = array();
		foreach ($rows as $row)
			$result[$row['PK']] = $row;
			
		return $this->_grouping->filter($result);
	}
	
	/**
	 * Statistics from the feedback feature
	 *
	 * Rating - The average rating in percent (0 <= r <= 100) 
	 * Max rating - The maxium average rating given that multiple ratings per issue exist
	 * Min rating - The minimum average rating given that multiple ratings per issue exist
	 *
	 * @return	array
	 */
	function getFeedbackStatistics() {
		$result = array();

		$i = $this->_query->join('synd_issue','i');
		$query = clone $this->_query;

		// Average rating
		$if = $query->join('synd_issue_feedback', 'if');
		$query->where("$i.node_id = $if.issue_node_id");
		$query->where("$i.info_status >= ".synd_node_issue::CLOSED." AND $i.info_status < ".synd_node_issue::MAX_STATUS_VALUE);
		
		$query->column($this->_grouping->getClosedKey($this, $query), 'PK');
		$query->column("SUM($if.info_rating)", 'RATING');
		$query->column("COUNT(1)", 'RATING_CNT');
		$query->groupBy($this->_grouping->getClosedKey($this, $query));
		
		$sql = $query->toString();
		$rows = $this->_db->getAll($sql);
		foreach ($rows as $row)
			$result[$row['PK']] = $row;
			
		// Max and min rating
		$query = clone $this->_query;

		$sql = "
				SELECT ift.issue_node_id, MAX(ift.info_rating) rmax, MIN(ift.info_rating) rmin
				FROM synd_issue_feedback ift 
				GROUP BY ift.issue_node_id";
		
		$if = $query->join("($sql)", 'if');
		$query->where("$i.node_id = $if.issue_node_id");
		$query->where("$i.info_status >= ".synd_node_issue::CLOSED." AND $i.info_status < ".synd_node_issue::MAX_STATUS_VALUE);
		
		$query->column($this->_grouping->getClosedKey($this, $query), 'PK');
		$query->column("SUM($if.rmax)", 'RATING_MAX');
		$query->column("SUM($if.rmin)", 'RATING_MIN');
		$query->column("COUNT(1)", 'RATING_DISTINCT_CNT');
		$query->groupBy($this->_grouping->getClosedKey($this, $query));
		
		$sql = $query->toString();
		$rows = $this->_db->getAll($sql);
		foreach ($rows as $row)
			$result[$row['PK']] = isset($result[$row['PK']]) ? array_merge($result[$row['PK']], $row) : $row;

		return $this->_grouping->filter($result);
	}
}

/**
 * Service level report grouping strategy
 * @access		public
 * @package		synd.core.module
 */
class ServiceLevelGrouping {
	function hasInstanceGroups() {
		return true;
	}
	
	function isSensitive() {
		return false;
	}

	function initialize($query) {}

	function filter($rows) {
		return $rows;
	}
	
	function getReceivedKey($report, $query) {
		return $query->join('synd_issue','i').'.node_id';
	}

	function getReceivedTaskKey($report, $query) {
		return $this->getReceivedKey($report, $query);
	}

	function getClosedKey($report, $query) {
		return $this->getReceivedKey($report, $query);
	}

	function getResolveByKey($report, $query) {
		return $this->getReceivedKey($report, $query);
	}

	function _group($rows, $callback) {
		$result = array();
		foreach ($rows as $pk => $stats) {
			$group = call_user_func($callback, $pk);
			if (!isset($result[$group])) 
				$result[$group] = $stats;
			else {
				if (is_numeric($stats))
					$result[$group] += $stats;
				else if (is_array($stats)) {
					foreach ($stats as $key => $value) {
						if (is_numeric($value))
							$result[$group][$key] = isset($result[$group][$key]) ? $result[$group][$key] + $value : $value;
					}
				}
			}
		}
		return $result;
	}
}

/**
 * Groups the report on project
 */
class ServiceLevelProjectGrouping extends ServiceLevelGrouping {
	function getReceivedKey($report, $query) {
		return $query->join('synd_issue','i').'.parent_node_id';
	}
}

/**
 * Groups the report on assigned user
 */
class ServiceLevelAssignedGrouping extends ServiceLevelGrouping {
	function isSensitive() {
		return true;
	}

	function getReceivedKey($report, $query) {
		return $query->join('synd_issue','i').'.assigned_node_id';
	}

	function getReceivedTaskKey($report, $query) {
		return $report->_taskQuery->join('synd_issue_task','t').'.create_node_id';
	}
}

/**
 * Groups the report on customer
 */
class ServiceLevelCustomerGrouping extends ServiceLevelGrouping {
	function isSensitive() {
		return true;
	}

	function getReceivedKey($report, $query) {
		return $query->join('synd_issue','i').'.client_node_id';
	}
}

/**
 * Groups the report on customer department
 */
class ServiceLevelDepartmentGrouping extends ServiceLevelCustomerGrouping {
	function filter($rows) {
		return $this->_group($rows, array($this, '_callback_department'));
	}

	function _callback_department($key) {
		if (null !== ($client = SyndNodeLib::getInstance($key))) {
			$department = $client->getOrganization();
			if (!$department->isNull())
				return $department->nodeId;
//			$email = $client->getEmail();
//			if (false !== ($i = strpos($email, '@')))
//				return 'case.'.substr($email, $i+1);
		}
		return null;
	}
}

/**
 * Groups the report on category
 */
class ServiceLevelCategoryGrouping extends ServiceLevelGrouping {
	function initialize($query) {
		$i = $query->join('synd_issue','i');
		$ik = $query->join('synd_issue_keyword', 'ik');
		$query->where("$i.node_id = $ik.issue_node_id");
	}
	
	function getReceivedKey($report, $query) {
		return $query->join('synd_issue_keyword','ik').'.keyword_node_id';
	}
}

/**
 * Groups the report on period to show trends
 */
class ServiceLevelPeriodGrouping extends ServiceLevelGrouping {
	var $_format = null;
	
	/**
	 * @see		strftime()
	 * @param	string	Date format to group by
	 */
	function ServiceLevelPeriodGrouping($format = '%Y-%m') {
		$this->_format = $format;
	} 

	function hasInstanceGroups() {
		return false;
	}

	function filter($rows) {
		return $this->_group($rows, array($this, '_callback_period'));
	}

	function _callback_period($key) {
		return null != $key ? strftime($this->_format, strtotime($key)) : '';
	}
	
	function getReceivedKey($report, $query) {
		return $report->_db->date($report->_db->unixtodate(
			$query->join('synd_issue','i').'.ts_create'));
	}

	function getReceivedTaskKey($report, $query) {
		return $report->_db->date($report->_db->unixtodate(
			$query->join('synd_issue_task','t').'.ts_create'));
	}

	function getClosedKey($report, $query) {
		return $report->_db->date($report->_db->unixtodate(
			$query->join('synd_issue','i').'.ts_resolve'));
	}

	function getResolveByKey($report, $query) {
		return $report->_db->date($report->_db->unixtodate(
			$query->join('synd_issue','i').'.ts_resolve_by'));
	}
}

