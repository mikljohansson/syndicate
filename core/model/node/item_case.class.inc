<?php
require_once 'core/model/node/case.class.inc';

class synd_node_item_case extends synd_node_case {
	function getSerial() {
		return '';
	}

	function getInternalSerial() {
		return $this->objectId();
	}

	function _public_getInternalSerial() {
		return $this->getInternalSerial();
	}

	function getModel() {
		return '';
	}

	function _public_getModel() {
		return $this->getModel();
	}
	
	function getCustomer() {
		return SyndNodeLib::getInstance('user_null.null');
	}
	
	function _setCustomer($client) {
		$result = null;
		return $result;
	}
}
