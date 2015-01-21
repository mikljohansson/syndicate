<?php
require_once 'core/model/workflow/WorkflowActivity.class.inc';

abstract class synd_workflow_instance extends AbstractWorkflowActivity {
	protected $_option = null;

	abstract function getOptions();

	function getDescription() {
		$options = $this->getOptions();
		if (isset($options[$this->_option]))
			return $options[$this->_option];
	}

	function setSelectedOption($option) {
		$this->_option = $option;
	}
	
	function getSelectedOption() {
		return SyndNodeLib::getInstance($this->_option);
	}

	function _view_setSelectedOption(Request $request, Template $page) {
		$this->setSelectedOption($request['option']);
	}
}
