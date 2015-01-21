<?php
require_once 'core/lib/SyndDate.class.inc';
require_once 'core/model/workflow/WorkflowActivity.class.inc';

/**
 * Adds a comment to an issue
 */
class synd_workflow_comment extends AbstractWorkflowActivity {
	protected $_comment = null;
	protected $_duration = null;
	
	function __construct() {
		parent::__construct();
		$this->_duration = '20';
	}
	
	function __toString() {
		return SyndLib::translate('Add comment to issue');
	}

	function getComment() {
		return $this->_comment;
	}
	
	function getDuration() {
		return $this->_duration;
	}

	function process(Workflow $workflow, $subjects, Response $response) {
		foreach ($subjects as $subject)
			$subject->addTask($this->_comment, SyndDate::evalDuration($this->_duration) * 60);
	}

	function _view_setComment(Request $request, Template $page) {
		$this->_comment = $request['comment'];
		$this->_duration = $request['duration'];
	}
}
