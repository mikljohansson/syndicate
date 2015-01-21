<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _vendor_OracleCalendar extends PHPUnit2_Framework_TestCase {
	function setUp() {
		require_once 'core/lib/calendar/VCalendar.class.inc';
		require_once 'core/lib/Mime.class.inc';
		require_once 'core/lib/vendor/OracleCalendar.class.inc';
	}
	
	function getConnection() {
		global $synd_config;
		$uri = parse_url($synd_config['db']['ocal']);
		return ocal_connect($uri['host'], strtr($uri['user'],'#','/'), $uri['pass']);
	}
	
	function testConnection() {
		$connection = $this->getConnection();
		$this->assertType('resource', $connection);
		
		$agenda = ocal_open_agenda($connection, 'mikl/test');
		$this->assertType('resource', $agenda);
		
		$ok = ocal_close_agenda($agenda);
		$this->assertTrue($ok);
		
		$ok = ocal_disconnect($connection);
		$this->assertTrue($ok);
	}
	
	function testEvents() {
		$connection = $this->getConnection();
		$agenda = ocal_open_agenda($connection, 'mikl/test');
		
		$ical = new VCalendarObject('VCALENDAR');
		$ical->setProperty('VERSION', '2.0');
		$ical->setProperty('PRODID', '-//Synd (www.synd.info)//Synd CSDK Extension 1.0//EN');
		
		$event = $ical->appendChild(new VCalendarEvent());
//		$event->setProperty('X-ORACLE-IMEETING-SENDEMAILNOTIFICATION', 'FALSE');
//		$event->setProperty('X-ORACLE-EVENTTYPE', 'APPOINTMENT');
		
//		$event->setProperty('TRANSP', 'OPAQUE');
		$event->setProperty('STATUS', 'CONFIRMED');
//		$event->setProperty('SEQUENCE', '1');
//		$event->setProperty('PRIORITY', '5');
		$event->setStart(time());
		$event->setEnd(time()+3600);
//		$event->setDateProperty('CREATED', time());
//		$event->setDateProperty('DTSTAMP', time());
		$event->setProperty('CLASS', 'PUBLIC');

		$event->setSummary('_unit_test: This event is safe to delete');
		$event->setLocation('Nowhere');
		$event->addAttendee("mailto:mikael+test@chl.chalmers.se");
		
		$body = new MimeTextpart($ical->toString(), '');
		$body->setHeader('Content-Type', 'text/calendar; charset=UTF-8');
		$body->setHeader('Content-Transfer-Encoding', 'QUOTED-PRINTABLE');
		$mime = new MimeMultipart();
		$mime->addPart($body);
		
		$uids = ocal_store_events($connection, $mime->toString());
		$this->assertFalse(empty($uids));
		$uid = reset($uids);
		
		$response = ocal_fetch_events_by_uid($connection, $agenda, $uids);
		$this->assertFalse(empty($response));
		
		$mimeResponse = Mime::parse($response);
		$icalResponse = VCalendar::parse($mime->getContent());
		$this->assertNotNull($icalResponse);
		if (null == $icalResponse) return;
		
		$eventResponse = reset($icalResponse->getChildren());
		$this->assertEquals($event->getStart(), $eventResponse->getStart());
		$this->assertEquals($event->getEnd(), $eventResponse->getEnd());
		
		$deleted = ocal_delete_events($connection, $uids);
		$this->assertTrue($deleted);

		$response = ocal_fetch_events_by_uid($connection, $agenda, $uids);
		$this->assertFalse($response);
	}

	function testConnectionObject() {
		global $synd_config;
		$connection = new OracleCalendar($synd_config['db']['ocal']);
		$user = SyndNodeLib::getInstance('user_case.mikl/test');
		$user->setEmail(strtr('mikael+test AT chl DOT chalmers DOT se',array(' AT '=>'@',' DOT '=>'.')));
		
		$agenda = $connection->getAgenda($user);
		$this->assertNotNull($agenda);
		
		if (null != $agenda) {
			$event = new VCalendarEvent(time(), time()+3600, '_unit_test: This event is safe to delete');
			$uid = $agenda->addEvent($event);
			$this->assertNotNull($uid);

			// Retrive event by UID
			$events = $agenda->getEventsByUID(array($uid));
			$this->assertFalse(empty($events));
			if (!empty($events)) {
				$eventResponse = reset($events);
				$this->assertEquals($uid, $eventResponse->getProperty('UID'));
			}

			// Retrive event by date range
			$events = $agenda->getEventsByRange(time()-1800, time()+1800);
			$this->assertFalse(empty($events));
			if (!empty($events)) {
				$eventResponse = reset($events);
				$this->assertEquals($uid, $eventResponse->getProperty('UID'));
			}

			// Delete event
			$ok = $connection->deleteEvent($uid);
			$this->assertTrue($ok);
			
			//foreach (array_keys($events) as $key)
			//	@$connection->deleteEvent($events[$key]->getProperty('UID'));
		}
	}
	
	function testCompositeAgenda() {
		global $synd_config;
		$connection = new OracleCalendar($synd_config['db']['ocal']);
		$user = SyndNodeLib::getInstance('user_case.mikl/test');
		$user->setEmail(strtr('mikael+test AT chl DOT chalmers DOT se',array(' AT '=>'@',' DOT '=>'.')));

		$agenda = $connection->getCompositeAgenda(array($user));
		$event = new VCalendarEvent(time(), time()+3600, '_unit_test: This event is safe to delete');
		$uid = $agenda->addEvent($event);
		$this->assertNotNull($uid);

		// Retrive event by UID
		$events = $agenda->getEventsByUID(array($uid));
		$this->assertFalse(empty($events));
		if (!empty($events)) {
			$eventResponse = reset($events);
			$this->assertEquals($uid, $eventResponse->getProperty('UID'));
		}

		// Retrive event by date range
		$events = $agenda->getEventsByRange(time()-1800, time()+1800);
		$this->assertFalse(empty($events));
		if (!empty($events)) {
			$eventResponse = reset($events);
			$this->assertEquals($uid, $eventResponse->getProperty('UID'));
		}

		// Delete event
		$ok = $connection->deleteEvent($uid);
		$this->assertTrue($ok);

		//foreach (array_keys($events) as $key)
		//	@$connection->deleteEvent($events[$key]->getProperty('UID'));
	}
}
