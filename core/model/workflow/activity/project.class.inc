<?php
require_once 'core/model/workflow/activity/instance.class.inc';

/**
 * Sets the project for an issue
 */
class synd_workflow_project extends synd_workflow_instance {
	protected $_option = null;
	
	function __toString() {
		return SyndLib::translate('Move to project');
	}

	function process(Workflow $workflow, $subjects, Response $response) {
		foreach ($subjects as $subject)
			$subject->setParent($this->getSelectedOption());
	}
	
	function getOptions() {
		global $synd_user;
		return SyndLib::sort(Module::getInstance('issue')->getProjectOptions($synd_user));
	}
}
