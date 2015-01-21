<?php
require_once 'core/model/workflow/WorkflowActivity.class.inc';

/**
 * Calls a method on the target object
 */
abstract class synd_workflow_method extends AbstractWorkflowActivity {
	protected $_option = null;
	
	abstract function getOptions();

	function getDescription() {
		$options = $this->getOptions();
		if (isset($options[$this->_option]))
			return $options[$this->_option];
		return '';
	}

	function setSelectedOption($option) {
		$this->_option = $option;
	}
	
	function getSelectedOption() {
		return $this->_option;
	}
	
	function _view_setSelectedOption(Request $request, Template $page) {
		$this->setSelectedOption($request['option']);
	}
}
