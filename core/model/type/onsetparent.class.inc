<?php
require_once "core/model/type/event.class.inc";

class synd_type_onsetparent extends synd_type_event {
	var $_project = null;
	
	function __construct($issue) {
		parent::__construct($issue);
		$project = $issue->getParent();
		$this->_project = $project->nodeId;
	}
	
	function toString() {
		return 'Moved';
	}
	
	function getProject() {
		return SyndNodeLib::getInstance($this->_project);
	}

	function getPrecedence() {
		return 50;
	}
}
