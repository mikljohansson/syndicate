<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/model/ISyncable.class.inc';
require_once 'core/lib/calendar/Calendar.class.inc';

class _design_CalendarWidget extends PHPUnit2_Framework_TestCase {
	function setUp() {
		require_once 'design/gui/CalendarWidget.class.inc';
	}
	
	function testRange() {
		$start = strtotime('2005-11-01 09:00');
		$stop = strtotime('2005-11-07 18:00');

		$widget = new CalendarWeekWidget();
		$widget->addEvent(new _UnitTestEvent($start+3600, $stop));
		$widget->addEvent(new _UnitTestEvent($start, $stop-3600));
		$this->assertEquals($start, $widget->getStart());
		$this->assertEquals($stop, $widget->getEnd());
		$this->assertEquals(3600*9, $widget->getDayStart());
		$this->assertEquals(3600*18, $widget->getDayEnd());
		
		$this->assertEquals(2, count($widget->getEvents()));
		$this->assertEquals(2, count($widget->getEvents($start, $stop)));
		$this->assertEquals(1, count($widget->getEvents($start, $start+1800)));
		$this->assertEquals(1, count($widget->getEvents($stop-1800, $stop)));
		$this->assertEquals(2, count($widget->getEvents($start+3600*2, $stop-3600*2)));
		$this->assertEquals(0, count($widget->getEvents($start-3600, $start-1800)));
		$this->assertEquals(1, count($widget->getEvents($start, $start+3600)));
		
		$widget = new CalendarWeekWidget($start, $stop);
		$widget->addEvent(new _UnitTestEvent($start-3600, $stop+3600));
		$this->assertEquals($start, $widget->getStart());
		$this->assertEquals($stop, $widget->getEnd());
		$this->assertEquals(3600*9, $widget->getDayStart());
		$this->assertEquals(3600*18, $widget->getDayEnd());
	}
	
	function testRendering() {
		$widget = new CalendarWeekWidget(strtotime('2005-11-01 08:15'), strtotime('2005-11-03 11:00'));
		$widget->addEvent(new _UnitTestEvent(strtotime('2005-11-01 09:45'), strtotime('2005-11-01 10:30'), 'Test'));
		$widget->addEvent(new _UnitTestEvent(strtotime('2005-11-02 09:15'), strtotime('2005-11-02 11:45'), 'Test2'));
		$widget->addEvent(new _UnitTestEvent(strtotime('2005-11-02 11:00'), strtotime('2005-11-02 13:00'), 'Test3'));
		
		$actual = $widget->toString();
		$expected = file_get_contents(dirname(__FILE__).'/_calendar/week-001-conflicts.html');
		$expected = str_replace('Tuesday', ucwords(strftime('%A',strtotime('2005-11-01'))), $expected);
		$expected = str_replace('Wednesday', ucwords(strftime('%A',strtotime('2005-11-02'))), $expected);
		$expected = str_replace('Thursday', ucwords(strftime('%A',strtotime('2005-11-03'))), $expected);
		
		$this->assertEquals(preg_replace('/\s*\n\s*/', '', $expected), preg_replace('/\s*\n\s*/', '', $actual));
	}
	
	function testResolution() {
		$widget = new CalendarWeekWidget(strtotime('2005-11-01 08:30'), strtotime('2005-11-01 11:00'), 1800);
		$widget->addEvent(new _UnitTestEvent(strtotime('2005-11-01 09:15'), strtotime('2005-11-01 10:30'), 'Test'));
	
		$actual = $widget->toString();
		$expected = file_get_contents(dirname(__FILE__).'/_calendar/week-002-resolution.html');
		$expected = str_replace('Tuesday', ucwords(strftime('%A',strtotime('2005-11-01'))), $expected);

		$this->assertEquals(preg_replace('/\s*\n\s*/', '', $expected), preg_replace('/\s*\n\s*/', '', $actual));
	}
}

class _UnitTestEvent implements CalendarEvent {
	var $_start = null;
	var $_stop = null;
	var $_title = null;
	
	function _UnitTestEvent($start, $stop, $title = null) {
		$this->_start = $start;
		$this->_stop = $stop;
		$this->_title = $title;
	}

	function getIdentifier() {}
	
	function getSummary() {
		return $this->_title;
	}

	function getDescription() {}
	function addAttendee($attendee) {}
	function getAttendees() {}

	function getStart() {
		return $this->_start;
	}

	function getEnd() {
		return $this->_stop;
	}
}
