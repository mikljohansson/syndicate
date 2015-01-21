<?php
require_once 'core/model/workflow/activity/method.class.inc';

/**
 * Adds a keyword to an issue
 */
class synd_workflow_status extends synd_workflow_method {
	function __toString() {
		return SyndLib::translate('Set status to');
	}

	function process(Workflow $workflow, $subjects, Response $response) {
		foreach ($subjects as $subject)
			$subject->setStatus($this->getSelectedOption());
	}
	
	function getOptions() {
		return SyndLib::array_collect(Module::getInstance('issue')->getDefinedStatusCodes(), 0);
	}
}
