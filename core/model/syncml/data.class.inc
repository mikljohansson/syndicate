<?php
require_once 'core/model/syncml/node.class.inc';

class synd_syncml_data extends synd_syncml_node {
	function synd_syncml_data($name = 'Data', $content = null) {
		parent::synd_syncml_node($name, $content);
	}
	
	function getContent() {
		if (false !== strpos($content = parent::getContent(), "\n"))
			return '<![CDATA['.$content.']]>';
		return $content;
	}
	
	function getMemento($format = null) {
		if ('text/x-vcalendar' == $format) {
			require_once 'core/lib/calendar/VCalendar.class.inc';
			return VCalendar::parse(parent::getContent());
		}
		return $this->getFirstChild();
	}
	
	function setMemento($memento) {
		assert('is_object($memento)');
		parent::setContent($memento->toString());
	}
}
