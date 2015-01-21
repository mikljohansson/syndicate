<?php
require_once 'core/model/workflow/activity/redirect.class.inc';

/**
 * Redirect to the edit dialog for the subject
 */
class synd_workflow_edit extends synd_workflow_redirect {
	function __toString() {
		return SyndLib::translate('Redirect to edit issue dialog');
	}

	function process(Workflow $workflow, $subjects, Response $response) {
		foreach ($subjects as $subject)
			$response->setLocation(tpl_view_jump($subject->getHandler(),'edit',$subject->nodeId));
	}
}
