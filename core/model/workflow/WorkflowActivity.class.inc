<?php
require_once 'core/Controller.class.inc';
require_once 'core/model/workflow/Workflow.class.inc';

/**
 * @access		public
 * @package		synd.core.model
 */
interface WorkflowActivity {
	/**
	 * Returns a description
	 * @return	string
	 */
	function __toString();

	/**
	 * Execute the activity
	 * @param	Workflow		Executing workflow instance
	 * @param	array			Objects to process
	 * @param	Response		Response to send back to client
	 */
	function process(Workflow $workflow, $subjects, Response $response);
	
	/**
	 * Returns the interface of supported input
	 * @return	string
	 */
	function getSupportedInput();
}

abstract class AbstractWorkflowActivity extends PublishedController implements WorkflowActivity {
	protected $_interface = null;
	
	function __construct($interface = 'DomainObject') {
		parent::__construct();
		$this->_interface = $interface;
	}
	
	function getSupportedInput() {
		return $this->_interface;
	}
}

class WorkflowActivityNull implements WorkflowActivity {
	function __toString() {}
	function process(Workflow $workflow, $subjects, Response $response) {}
	function getSupportedInput() {}
}
