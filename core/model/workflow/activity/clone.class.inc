<?php
require_once 'core/model/workflow/activity/sequence.class.inc';

/**
 * Calls a method on the target object
 */
class synd_workflow_clone extends synd_workflow_sequence {
	protected $_issue = null;
	protected $_number = null;
	
	function __toString() {
		return SyndLib::translate('Create issue from template');
	}

	function getNumber() {
		return $this->_number;
	}
	
	function setNumber($number) {
		$this->_number = $number;
		if (null != ($issue = synd_node_issue::findIssue(SyndNodeLib::getDefaultStorage('issue'), $this->_number)))
			$this->_issue = $issue->id();
	}
	
	function _view_setNumber(Request $request, Template $page) {
		$this->setNumber((int)$request['number']);
	}
	
	function getIssue() {
		if (null != ($issue = SyndLib::getInstance($this->_issue)))
			return $issue;
		return SyndNodeLib::getInstance('null.null');
	}

	function process(Workflow $workflow, $subjects, Response $response) {
		$proto = $this->getIssue();
		if ($proto->isNull()) 
			throw new InvalidArgumentException();
		
		$issue = clone $proto;
		$issue->checkout();
		
		parent::process($workflow, array($issue), $response);
	}
}
