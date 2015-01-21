<?php
require_once "core/model/type/event.class.inc";

class synd_type_onpause extends synd_type_event {
	function toString() {
		return 'Set as pending';
	}

	function getPrecedence() {
		return 30;
	}
}
