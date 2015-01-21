<?php
require_once 'core/model/DomainEvent.class.inc';

class synd_event_create extends AbstractDomainEvent {
	function __toString() {
		return 'Create';
	}
	
	function getDescription() {
		return 'When a new item is created';
	}
}
