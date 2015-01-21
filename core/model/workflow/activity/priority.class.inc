<?php
require_once 'core/model/workflow/activity/method.class.inc';

/**
 * Adds a keyword to an issue
 */
class synd_workflow_priority extends synd_workflow_method {
	function __toString() {
		return SyndLib::translate('Set priority');
	}

	function process(Workflow $workflow, $subjects, Response $response) {
		foreach ($subjects as $subject)
			$subject->setPriority($this->getSelectedOption());
	}
	
	function getOptions() {
		return Module::getInstance('issue')->getDefinedPriorities();
	}
}
