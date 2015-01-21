<?php
require_once 'core/model/workflow/WorkflowActivity.class.inc';

class synd_workflow_href extends AbstractWorkflowActivity {
	protected $_uri = null;
	
	function __toString() {
		return SyndLib::translate('Surf to location');
	}
	
	function process(Workflow $workflow, $subjects, Response $response) {
		$response->setLocation($this->_uri);
	}
	
	function getLocation() {
		return $this->_uri;
	}
	
	function setLocation($uri) {
		$this->_uri = $uri;
	}
	
	function _view_setLocation(Request $request, Template $page) {
		$this->setLocation(substr($request['uri'], 0, 2048));
	}
}
