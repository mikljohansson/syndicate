<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/lib/calendar/VCalendar.class.inc';

class _lib_VCalender extends PHPUnit2_Framework_TestCase {
	function testParse() {
		$summary = Mime::charset("Mikael едц", 'iso-8859-1');
		$description = "You should call Mikael, \r\nas soon as you get in ";
		$comment = "Test line folding and unfolding with some amount of whitespace at the end of the line";
		
		$expected = new VCalendarObject();
		$expected->setProperty('VERSION', '1.0');
		
		$todo = $expected->appendChild(new VCalendarTodo());

		$todo->setProperty('SUMMARY', $summary, VCalendar::QUOTED_PRINTABLE|VCalendar::UTF8);
		$todo->setProperty('DESCRIPTION', $description, VCalendar::QUOTED_PRINTABLE);
		$todo->setProperty('COMMENT', "Multiple lines \r\nline 1 \r\nline 2", VCalendar::QUOTED_PRINTABLE);
		$todo->setComment($comment);
		$todo->setProperty('CLASS', 'PUBLIC');
		$todo->setDue(strtotime('2005-01-17 16:00:00Z'));
		$todo->setStart(strtotime('2005-01-16 08:00:00Z'));
		$todo->setProperty('PRIORITY', '2');
		$todo->setProperty('STATUS', 'NEEDS ACTION');
		$todo->setAlarm(strtotime('2005-01-17 07:00:00Z'), 'Call Mikael');
		
		$buffer = file_get_contents(dirname(__FILE__).'/_vcal/ical-001-vtodo.ics');
		$actual = VCalendar::parse($buffer);
		
		$this->assertEquals($expected->toString(), $actual->toString());
		$this->assertEquals($expected, $actual);

		if (!count($children = $actual->getChildren()))
			return;
		$actual = reset($children);
		
		$this->assertEquals($summary, $actual->getSummary());
		$this->assertEquals($summary, $todo->getSummary());
		
		$this->assertEquals($description, $actual->getDescription());
		$this->assertEquals($description, $todo->getDescription());

		$this->assertEquals($summary, quoted_printable_decode(Mime::quotedPrintableEncode($summary)));
		$this->assertEquals($description, quoted_printable_decode(Mime::quotedPrintableEncode($description)));
		$this->assertEquals($comment, quoted_printable_decode(Mime::quotedPrintableEncode($comment)));
	}
	
	function testProperty() {
		$calendar = VCalendar::parse(file_get_contents(dirname(__FILE__).'/_vcal/ical-001-vtodo.ics'));
		$children = $calendar->getChildren();
		$todo = reset($children);
		$this->assertNotNull($todo);
		if (null == $todo)
			return;
		
		$summary = Mime::charset("Mikael едц", 'iso-8859-1');
		$this->assertEquals($summary, $todo->getProperty('SUMMARY'));
		$this->assertEquals(strtotime('2005-01-17 16:00:00Z'), $todo->getDue());
		$this->assertEquals(strtotime('2005-01-16 08:00:00Z'), $todo->getStart());
	}
	
	function testSynthesis() {
		$buffer = file_get_contents(dirname(__FILE__).'/_vcal/ical-002-vtodo.ics');
		$memento = VCalendar::parse($buffer);
		$children = $memento->getChildren();
		$actual = reset($children);
		$this->assertEquals('None', trim($actual->getDescription()));
	}

	function testQuotedValues() {
		$buffer = "BEGIN:VCALENDAR
FOO:\\:\\;\\,\\\\
END:VCALENDAR
";
		$memento = VCalendar::parse($buffer);
		$this->assertEquals(':;,\\', $memento->getProperty('FOO'));

		$memento = new VCalendarObject();
		$memento->setProperty('FOO', ':;,\\');
		$actual = $memento->toString();
		$this->assertEquals($buffer, $actual);
	}
}