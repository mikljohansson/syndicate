<?php
require_once 'core/model/workflow/WorkflowActivity.class.inc';

/**
 * Calls a method on the subject with a string parameter
 */
abstract class synd_workflow_text extends AbstractWorkflowActivity {
	protected $_option = null;
	
	/**
	 * @param	string	Interface to filter (DomainObject)
	 * @param	string	Default value of parameter
	 */
	function __construct($interface, $defval = '') {
		parent::__construct($interface);
		$this->_option = $defval;
	}
	
	function getDescription() {
		return $this->_option;
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
