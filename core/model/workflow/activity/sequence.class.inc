<?php
require_once 'core/model/workflow/WorkflowActivity.class.inc';

class synd_workflow_sequence extends AbstractWorkflowActivity {
	protected $_activities = array();
	protected $_protos = null;
	
	function __toString() {
		return 'Sequence of activities';
	}
	
	function addActivity(WorkflowActivity $activity) {
		$this->_activities[] = $activity;
	}
	
	function _view_addActivity(Request $request, Template $page) {
		if ('' === $request['activity'])
			return null;
		if (!isset($this->_protos[$request['activity']]))
			throw new InvalidArgumentException();
		$this->_activities[] = clone $this->_protos[$request['activity']];
	}
	
	function _view_removeActivity(Request $request, Template $page) {
		if (isset($this->_activities[$request[0]]))
			unset($this->_activities[$request[0]]);
	}
	
	function getActivities() {
		return $this->_activities;
	}
	
	function getActivityOptions(Workflow $workflow) {
		return $this->_protos = (array)SyndLib::runHook('workflow_activity_protos', $workflow);
	}
	
	function process(Workflow $workflow, $subjects, Response $response) {
		foreach ($this->_activities as $activity)
			$activity->process($workflow, $subjects, $response);
	}
}
