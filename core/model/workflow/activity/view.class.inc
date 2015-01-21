<?php
require_once 'core/model/workflow/activity/redirect.class.inc';

/**
 * Redirect to the view dialog for the subject
 */
class synd_workflow_view extends synd_workflow_redirect {
	function __toString() {
		return SyndLib::translate('Redirect to view issue dialog');
	}

	function process(Workflow $workflow, $subjects, Response $response) {
		foreach ($subjects as $subject)
			$response->setLocation(tpl_view($subject->getHandler(),$subject->objectId()));
	}
}
