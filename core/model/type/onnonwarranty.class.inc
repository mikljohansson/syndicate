<?php
require_once "core/model/type/event.class.inc";

class synd_type_onnonwarranty extends synd_type_event {
	function toString() {
		return 'Set as non-warranty';
	}

	function getPrecedence() {
		return 25;
	}
}
