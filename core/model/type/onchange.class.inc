<?php
require_once "core/model/type/event.class.inc";

class synd_type_onchange extends synd_type_event {
	function toString() {
		return 'Changed';
	}
}
