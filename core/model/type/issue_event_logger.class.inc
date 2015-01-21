<?php

class synd_type_issue_event_logger extends SyndType {
	var $_events = array();
	
	function _hook_loadInstance($id) {
		foreach (array_keys($this->_events) as $key) {
			if ($this->_events[$key]->getInstanceId() == $id)
				return $this->_events[$key];
		}
		$result = null;
		return $result;
	}
	
	function handleEvent($event) {
		$this->_events[] = $event;
	}
	
	function getEvents() {
		$events = array();
		foreach ($this->_events as $event) {
			if (null == ($end = end($events)) || abs($end->getTimestamp() - $event->getTimestamp()) > 2)
				array_push($events, $event);
			else if ($event->getPrecedence() > $end->getPrecedence()) {
				array_pop($events);
				array_push($events, $event);
			}
			else if ($event->getPrecedence() == $end->getPrecedence())
				array_push($events, $event);
		}
		return array_reverse($events);
	}
}
