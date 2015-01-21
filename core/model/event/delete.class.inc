<?php
require_once 'core/model/DomainEvent.class.inc';

class synd_event_delete extends AbstractDomainEvent {
	function __toString() {
		return 'Delete';
	}

	function getDescription() {
		return 'When an item is deleted';
	}
}
