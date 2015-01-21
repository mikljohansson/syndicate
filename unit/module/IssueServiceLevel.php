<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _modules_IssueServiceLevel extends PHPUnit2_Framework_TestCase {
	var $_storage = null;
	var $_db = null;
	var $_project = null;
	var $_issue = null;
	
	function setUp() {
		require_once 'core/module/issue/ServiceLevelReport.class.inc';
		$this->_storage = SyndNodeLib::getDefaultStorage('project');
		$persistent = $this->_storage->getPersistentStorage();
		$this->_db = $persistent->getDatabase();
		
		$this->_project = $this->_storage->factory('project');

		$this->_project->_db->query("
			DELETE FROM synd_issue
			WHERE parent_node_id IN (
				SELECT p.node_id FROM synd_project p 
				WHERE p.create_node_id = 'case._unit_test')");

		$this->_project->_db->query("
			DELETE FROM synd_project
			WHERE create_node_id = 'case._unit_test'");
		
		$this->_project->setTitle('_unit_test: This project is safe to delete');
		$this->_project->data['CREATE_NODE_ID'] = 'case._unit_test';
		$this->_project->save();
		
		$this->_issue = $this->_storage->factory('issue');
		$this->_issue->setParent($this->_project);
		$this->_issue->setCustomer(SyndNodeLib::getInstance('user_case._unit_test'));
		$this->_issue->setTitle('_unit_test: This issue is safe to delete');
		$this->_issue->data['TS_RESOLVE_BY'] = time()-60;
		$this->_issue->save();
		
		$this->_storage->flush();
	}
	
	function tearDown() {
		$this->_issue->delete();
		$this->_issue->flush();
		
		$sql = "
			DELETE FROM synd_issue
			WHERE parent_node_id = ".$this->_project->_db->quote($this->_project->nodeId);
		$this->_project->_db->query($sql);
		
		$this->_project->delete();
		$this->_project->flush();
	}
	
	function testReceivedIssues() {
		// Basic project report
		$report = new ServiceLevelReport($this->_storage, $this->getProjectQuery(), new ServiceLevelProjectGrouping());
		$expected = array($this->_project->nodeId => '1');
		$actual = $report->getReceivedIssues();
		$this->assertEquals($expected, $actual);

		// Test child project reports
		$child = $this->_project($this->_project);
	
		$issue = $child->appendChild($this->_project->_storage->factory('issue'));
		$issue->setCustomer(SyndNodeLib::getInstance('user_case._unit_test'));
		$issue->setTitle('_unit_test: This issue is safe to delete');
		$issue->data['INFO_PRIO'] = 0;
		$issue->save();
		$issue->flush();
		
		$actual = $report->getReceivedIssues();
		$this->assertEquals($expected, $actual);

		$report = new ServiceLevelReport($this->_storage, $this->getRecursiveQuery(), new ServiceLevelProjectGrouping());
		$expected = array(
			$this->_project->nodeId => '1',
			$child->nodeId => '1');
		$actual = $report->getReceivedIssues();
		$this->assertEquals($expected, $actual);

		$issue->delete();
		$child->_mailNotifier->_active = array();
		$child->delete();
		$child->_storage->flush();

		// Test trends grouping
		$report = new ServiceLevelReport($this->_storage, $this->getProjectQuery(), new ServiceLevelPeriodGrouping('%Y-%m'));
		$expected = array(strftime('%Y-%m') => '1');
		$actual = $report->getReceivedIssues();
		$this->assertEquals($expected, $actual);

		// Test client grouping 
		$report = new ServiceLevelReport($this->_storage, $this->getProjectQuery(), new ServiceLevelCustomerGrouping());
		$expected = array('user_case._unit_test' => '1');
		$actual = $report->getReceivedIssues();
		$this->assertEquals($expected, $actual);

		// Test department grouping 
		$report = new ServiceLevelReport($this->_storage, $this->getProjectQuery(), new ServiceLevelDepartmentGrouping());
		$expected = array('' => '1');
		$actual = $report->getReceivedIssues();
		$this->assertEquals($expected, $actual);
	}
	
	function testReceivedSecondLine() {
		$issue = $this->_issue($this->_project);
		
		$child = $this->_project($this->_project);
		$issue2 = $this->_issue($child);
		$issue2->setParentIssue($issue);
		$issue2->save();
		$issue2->flush();
		
		// Branch project report
		$report = new ServiceLevelReport($this->_storage, $this->getRecursiveQuery(), new ServiceLevelProjectGrouping());
		$expected = array(
			$this->_project->nodeId => '2',
			$child->nodeId => '1'
			);
		$actual = $report->getReceivedIssues();
		$this->assertEquals($expected, $actual);

		$expected = array($this->_project->nodeId => '1');
		$actual = $report->getReceivedSecondLine();
		$this->assertEquals($expected, $actual);

		$issue2->delete();
		$issue->delete();
		$child->_mailNotifier->_active = array();
		$child->delete();
	}
	
	function testClosedIssues() {
		// Basic project report
		$report = new ServiceLevelReport($this->_storage, $this->getProjectQuery(), new ServiceLevelProjectGrouping());
		$actual = $report->getClosedIssues();
		$this->assertEquals(array(), $actual);

		$this->_issue->data['TS_CREATE'] = strtotime('-15 minutes');
		$this->_issue->setStatus(synd_node_issue::CLOSED);
		$this->_issue->save();
		$this->_issue->flush();
		
		$expected = array($this->_project->nodeId => '1');
		$actual = $report->getClosedIssues();
		$this->assertEquals($expected, $actual);
		
		// Test closed interval
		$expected = array($this->_project->nodeId => '1');
		$actual = $report->getClosedIssues(60*20);
		$this->assertEquals($expected, $actual);
		
		$expected = array();
		$actual = $report->getClosedIssues(60*10);
		$this->assertEquals($expected, $actual);
	}
	
	function testPendingIssues() {
		// Basic project report
		$report = new ServiceLevelReport($this->_storage, $this->getProjectQuery(), new ServiceLevelProjectGrouping());

		$expected = array($this->_project->nodeId => '1');
		$actual = $report->getPendingIssues();
		$this->assertEquals($expected, $actual);

		$this->_issue->setStatus(synd_node_issue::CLOSED);
		$this->_issue->save();
		$this->_issue->flush();
		
		$actual = $report->getPendingIssues();
		$this->assertEquals(array(), $actual);
	}

	function testPendingSecondLine() {
		$issue = $this->_issue($this->_project);
		
		$child = $this->_project($this->_project);
		$issue2 = $this->_issue($child);
		$issue2->setParentIssue($issue);
		$issue2->save();
		$issue2->flush();
		
		// Branch project report
		$report = new ServiceLevelReport($this->_storage, $this->getRecursiveQuery(), new ServiceLevelProjectGrouping());
		$expected = array($this->_project->nodeId => '1');
		$actual = $report->getPendingSecondLine();
		$this->assertEquals($expected, $actual);

		$issue2->setStatus(synd_node_issue::CLOSED);
		$issue2->save();
		$issue2->flush();
		
		$actual = $report->getPendingSecondLine();
		$this->assertEquals(array(), $actual);

		$issue2->delete();
		$issue->delete();
		$child->_mailNotifier->_active = array();
		$child->delete();
	}
	
	function testLoggedTime() {
		$task = SyndNodeLib::factory('task');
		$task->setParent($this->_issue);
		$task->data['INFO_DURATION'] = 20;
		$task->save();
		$task->flush();
		
		// Basic project report
		$report = new ServiceLevelReport($this->_storage, $this->getProjectQuery(), new ServiceLevelProjectGrouping());
		$expected = array(
			$this->_project->nodeId => array(
				'PK' => $this->_project->nodeId,
				'ISSUES' => '1',
				'TIME_LOGGED' => '1200')
			);
		$actual = $report->getLoggedTime();
		$this->assertEquals($expected, $actual);

		$task->delete();
	}

	function testEstimatedTime() {
		$this->_issue->data['INFO_ESTIMATE'] = 60;
		$this->_issue->save();
		$this->_issue->flush();
		
		$task = SyndNodeLib::factory('task');
		$task->setParent($this->_issue);
		$task->data['INFO_DURATION'] = 20;
		$task->save();
		$task->flush();
		
		// Basic project report
		$report = new ServiceLevelReport($this->_storage, $this->getProjectQuery(), new ServiceLevelProjectGrouping());
		$expected = array(
			$this->_project->nodeId => array(
				'PK' => $this->_project->nodeId,
				'ISSUES' => '1',
				'TIME_ESTIMATE' => '3600',
				'TIME_LOGGED' => '1200')
			);
		$actual = $report->getEstimatedTime();
		$this->assertEquals($expected, $actual);

		$task->delete();
	}

	function testPerformanceStatistics() {
		$this->_issue->setStatus(synd_node_issue::CLOSED);
		$this->_issue->data['TS_START'] = $this->_issue->data['TS_CREATE']+30;
		$this->_issue->data['TS_RESOLVE'] = $this->_issue->data['TS_CREATE']+60;
		$this->_issue->data['TS_RESOLVE_BY'] = $this->_issue->data['TS_CREATE']+70;
		$this->_issue->save();
		$this->_issue->flush();
		
		// Basic project report
		$report = new ServiceLevelReport($this->_storage, $this->getProjectQuery(), new ServiceLevelProjectGrouping());
		$expected = array(
			$this->_project->nodeId => array(
				'PK' => $this->_project->nodeId,
				'TIME_TO_SOLUTION' => '60',
				'TIME_TO_ACTIVE' => '30',
				'ACCURACY' => '10')
			);
		$actual = $report->getPerformanceStatistics();
		$this->assertEquals($expected, $actual);
	}
	
	function testFeedbackStatistics() {
		$token1 = substr(md5(($id = abs(crc32(uniqid('')))).$this->_issue->getPrivateKey()),0,16).dechex($id);
		$token2 = substr(md5(($id = abs(crc32(uniqid('')))).$this->_issue->getPrivateKey()),0,16).dechex($id);
		$token3 = substr(md5(($id = abs(crc32(uniqid('')))).$this->_issue->getPrivateKey()),0,16).dechex($id);
		
		$this->_issue->_view_feedback(array(2 => $token1, 3 => 1));
		$this->_issue->_view_feedback(array(2 => $token2, 3 => 1));
		$this->_issue->_view_feedback(array(2 => $token3, 3 => 0));
		$this->_issue->setStatus(synd_node_issue::CLOSED);
		$this->_issue->save();
		$this->_issue->flush();

		// Basic project report
		$report = new ServiceLevelReport($this->_storage, $this->getProjectQuery(), new ServiceLevelProjectGrouping());
		$expected = array(
			$this->_project->nodeId => array(
				'PK' => $this->_project->nodeId,
				'RATING' => '2',
				'RATING_CNT' => '3',
				'RATING_MAX' => '1',
				'RATING_MIN' => '0',
				'RATING_DISTINCT_CNT' => '1')
			);
		$actual = $report->getFeedbackStatistics();
		$this->assertEquals($expected, $actual);
	}
	
	function _project($project) {
		$child = $project->_storage->factory('project');
		$child->setParent($project);
		$child->setTitle('_unit_test: This project is safe to delete');
		$child->data['CREATE_NODE_ID'] = 'case._unit_test';
		$child->save();
		$project->_storage->flush();
		return $child;
	}
	
	function _issue($project, $client = 'user_case._unit_test') {
		$issue = $project->_storage->factory('issue');
		$issue->setParent($project);
		$issue->setCustomer(SyndNodeLib::getInstance($client));
		$issue->setTitle('_unit_test: This issue is safe to delete');
		$issue->data['TS_RESOLVE_BY'] = time()-60;
		$issue->save();
		$project->_storage->flush();
		return $issue;
	}

	function getProjectQuery() {
		$query = $this->_db->createQuery();
		$query->where($query->join('synd_issue', 'i').'.parent_node_id = '.$this->_db->quote($this->_project->nodeId));
		return $query;
	}
	
	function getRecursiveQuery() {
		$ids = array_merge(array($this->_project->nodeId), SyndLib::collect($this->_project->getProjects(),'nodeId'));
		$query = $this->_db->createQuery();
		$query->where($query->join('synd_issue', 'i').'.parent_node_id IN ('.implode(',',$this->_db->quote($ids)).')');
		return $query;
	}
}
