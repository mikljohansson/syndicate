<?php
require_once 'core/model/workflow/WorkflowActivity.class.inc';

/**
 * Calls the save method on the subject
 */
class synd_workflow_save extends AbstractWorkflowActivity {
	protected $_option = null;
	
	function __toString() {
		return SyndLib::translate('Save to database');
	}

	function process(Workflow $workflow, $subjects, Response $response) {
		foreach ($subjects as $subject)
			$subject->save();
	}
}
