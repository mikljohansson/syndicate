<?php
require_once 'core/model/workflow/activity/sequence.class.inc';

/**
 * Calls a method on the target object and verifies the return value
 */
class synd_workflow_condition extends synd_workflow_sequence {
	protected $_option = null;
	
	function getDescription() {
		$options = getOptions();
		if (isset($options[$this->_option]))
			return $options[$this->_option];
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
