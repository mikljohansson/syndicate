<?php
require_once 'core/model/type/event.class.inc';

class synd_type_onassign extends synd_type_event {
	var $_assigned = null;
	
	function __construct($parent) {
		parent::__construct($parent);
		$assigned = $parent->getAssigned();
		$this->_assigned = $assigned->nodeId;
	}
	
	function toString() {
		return 'Assigned';
	}
	
	function getAssigned() {
		return SyndNodeLib::getInstance($this->_assigned);
	}

	function getPrecedence() {
		return 50;
	}
}
