<?php
require_once "core/model/type/event.class.inc";

class synd_type_oncancel extends synd_type_event {
	function toString() {
		return 'Canceled';
	}
	
	function getPrecedence() {
		return 50;
	}
}
