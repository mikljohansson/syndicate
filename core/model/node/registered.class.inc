<?php
require_once 'core/model/node/relation.class.inc';

class synd_node_registered extends synd_node_relation {
	function getCourse() {
		return $this->getParent();
	}
	
	function setCourse($course) {
		$this->setParent($course);
	}

	function getCustomer() {
		return $this->getChild();
	}

	function setCustomer($client) {
		$this->setChild($client);
	}
}
