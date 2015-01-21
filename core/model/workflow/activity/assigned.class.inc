<?php
require_once 'core/model/workflow/activity/instance.class.inc';

/**
 * Sets the assigned user for an issue
 */
class synd_workflow_assigned extends synd_workflow_instance {
	protected $_option = null;
	
	function __toString() {
		return SyndLib::translate('Assign to');
	}

	function process(Workflow $workflow, $subjects, Response $response) {
		foreach ($subjects as $subject)
			$subject->setAssigned($this->getSelectedOption());
	}
	
	function getOptions() {
		return array_merge(
			array('user_null.null' => SyndLib::translate('Unassigned')), 
			array('user_case.current' => SyndLib::translate('Logged in user')), 
			SyndLib::sort(Module::getInstance('issue')->getAssignedOptions()));
	}
	
	function getSelectedOption() {
		global $synd_user;
		if ('user_case.current' == $this->_option)
			return $synd_user;
		return SyndNodeLib::getInstance($this->_option);
	}
}
