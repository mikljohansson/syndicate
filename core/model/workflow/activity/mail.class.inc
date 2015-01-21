<?php
require_once 'core/model/workflow/WorkflowActivity.class.inc';

/**
 * Adds a keyword to an issue
 */
class synd_workflow_mail extends AbstractWorkflowActivity {
	protected $_customer = false;
	protected $_assigned = false;
	protected $_cc = null;
	
	function __toString() {
		return SyndLib::translate('E-mail to');
	}

	function isCustomerSelected() {
		return $this->_customer;
	}

	function isAssignedSelected() {
		return $this->_assigned;
	}
	
	function getCc() {
		return $this->_cc;
	}

	function process(Workflow $workflow, $subjects, Response $response) {
		foreach ($subjects as $subject) {
			$mailNotifier = $subject->getMailNotifier();

			if ($this->isCustomerSelected())
				$mailNotifier->addTemporary($subject->getCustomer());

			if ($this->isAssignedSelected())
				$mailNotifier->addTemporary($subject->getAssigned());

			foreach (preg_split('/[^\w\-+.@]+/', $this->getCc()) as $email) {
				$mailNotifier->addTemporaryEmail($email);
			}
		}
	}
	
	function _view_setSelectedOption(Request $request, Template $page) {
		$this->_customer = !empty($request['customer']);
		$this->_assigned = !empty($request['assigned']);
		$this->_cc = $request['cc'];
	}
}
