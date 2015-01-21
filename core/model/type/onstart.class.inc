<?php
require_once "core/model/type/event.class.inc";

class synd_type_onstart extends synd_type_event {
	function toString() {
		return 'Set as in progress';
	}

	function getPrecedence() {
		return 30;
	}
}
