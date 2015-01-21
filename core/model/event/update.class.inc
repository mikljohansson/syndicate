<?php
require_once 'core/model/DomainEvent.class.inc';

class synd_event_update extends AbstractDomainEvent {
	function __toString() {
		return 'Update';
	}

	function getDescription() {
		return 'Every time an item is modified';
	}
}
