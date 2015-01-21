<?php
require_once 'core/model/workflow/activity/condition.class.inc';

/**
 * Calls a method on the target object and verifies the return value
 */
class synd_workflow_ifstatus extends synd_workflow_condition {
	protected $_option = null;
	
	function __toString() {
		return SyndLib::translate('If status equals');
	}

	function getDescription() {
		$options = $this->getOptions();
		if (isset($options[$this->_option]))
			return $options[$this->_option];
	}

	function process(Workflow $workflow, $subjects, Response $response) {
		$matches = array();
		foreach ($subjects as $subject) {
			if ($subject->getStatus() == $this->_option)
				$matches[] = $subject;
		}

		parent::process($workflow, $matches, $response);
	}
	
	function getOptions() {
		return SyndLib::array_collect(Module::getInstance('issue')->getDefinedStatusCodes(), 0);
	}
	
	function setSelectedOption($option) {
		$this->_option = $option;
	}
	
	function getSelectedOption() {
		return $this->_option;
	}
	
	function _view_setSelectedOption(Request $request, Template $page) {
		$options = $this->getOptions();
		if (isset($request['option'], $options[$request['option']]))
			$this->setSelectedOption($request['option']);
	}
}
