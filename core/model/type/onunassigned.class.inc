<?php
require_once "core/model/type/event.class.inc";

class synd_type_onunassigned extends synd_type_event {
	function toString() {
		return 'Set as unassigned';
	}

	function getPrecedence() {
		return 30;
	}
}
