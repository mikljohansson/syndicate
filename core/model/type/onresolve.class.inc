<?php
require_once "core/model/type/event.class.inc";

class synd_type_onresolve extends synd_type_event {
	function toString() {
		return 'Closed';
	}

	function getPrecedence() {
		return 60;
	}
}
