<?php
require_once "core/model/type/event.class.inc";

class synd_type_oncreate extends synd_type_event {
	function toString() {
		return 'Created';
	}

	function getPrecedence() {
		return 20;
	}
}
