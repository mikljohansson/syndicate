<?php
require_once 'core/lib/calendar/Calendar.class.inc';
require_once 'core/lib/calendar/VCalendar.class.inc';
require_once 'core/lib/Mime.class.inc';

/**
 * Adapts an OracleCalendar to the Calendar interface
 * @access		public
 * @package		synd.core.lib.vendor
 */
class synd_calendar_ocal implements Calendar {
	private $_uri = null;
	private $_sysop = false;
	private $_connection = null;
	private $_facade = null;
	
	static function factory($driver, $dsn) {
		return new $driver($dsn);
	}
	
	/**
	 * @param	string	Server URI (ex. 'ocal://user:password@calendar.example.com?sysop=1')
	 */
	function __construct($uri) {
		$this->_uri = $uri;
	}

	protected function getConnection() {
		if (null == $this->_connection) {
			profiler_enter(__METHOD__);

			$info = parse_url($this->_uri);
			if (isset($info['query']))
				parse_str($info['query'], $args);
			else
				$args = null; 
			
			if (!empty($args['sysop'])) {
				$this->_connection = new OracleSysopCalendar($info['host'], 
					isset($info['user']) ? strtr($info['user'],'#','/') : '10000', 
					isset($info['pass']) ? $info['pass'] : '');
				$this->_sysop = true;
			}
			else {
				$this->_connection = new OracleCalendar($info['host'], 
					isset($info['user']) ? strtr($info['user'],'#','/') : '', 
					isset($info['pass']) ? $info['pass'] : '');
			}
			
			profiler_leave(__METHOD__);
		}
		return $this->_connection;
	}

	protected function getFacade() {
		if (null == $this->_facade)
			$this->_facade = new OracleCalendarFacade($this->getConnection());
		return $this->_facade;
	}

	/**
	 * Sets the current user
	 *
	 * Requires a SYSOP logon or use of the "uniaccessrights" command line 
	 * tool to give the current user priviledges to the target account.
	 *
	 * @param	User	The user to run as
	 * @return	bool		Returns true on success, false on failure
	 */
	function setIdentity($user) {
		$connection = $this->getConnection();
		return $this->_sysop && $connection->setIdentity($user->getLogin());
	}

	function getAgenda($user) {
		return new OracleCalendarAgendaAdapter(new OracleCalendarAgendaFacade(new OracleCalendarAgenda($this->getConnection(), $user->getLogin())));
	}

	function getCompositeAgenda($users) {
		$agenda = new OracleCalendarCompositeAgenda($this->getConnection());
		foreach ($users as $user)
			$agenda->addAgenda(new OracleCalendarAgenda($this->getConnection(), $user->getLogin()));
		return new OracleCalendarCompositeAgendaAdapter(new OracleCalendarCompositeAgendaFacade($agenda));
	}
	
	function addEvent($event) {
		return $this->getFacade()->storeEvent($event);
	}

	function addEvents($events) {
		return $this->getFacade()->storeEvents($events);
	}

	function deleteEvent($uid) {
		return $this->getFacade()->deleteEvent($uid);
	}
	
	function deleteEvents($uids) {
		return $this->getFacade()->deleteEvents($uids);
	}
}

/**
 * Enables communication with Oracle Calendar using VCalendar objects
 * @access		public
 * @package		synd.core.lib.vendor
 */
class OracleCalendarFacade {
	protected $_connection = null;
	
	/**
	 * @param	OracleCalendar	Calendar connection to wrap
	 */
	function __construct(OracleCalendar $connection) {
		$this->_connection = $connection;
	}
	
	/**
	 * Agenda factory
	 * @return	OracleCalendarAgendaFacade
	 */
	function getAgenda($username) {
		return new OracleCalendarAgendaFacade(new OracleCalendarAgenda($this->_connection, $username));
	}
	
	function getCapabilities($capabilityid) {
		return $this->_connection->getCapabilities($capabilityid);
	}

	function storeEvent(VCalendarObject $event, $flags = OracleCalendar::IMPORT) {
		if (is_array($ids = $this->storeEvents(array($event))))
			return reset($ids);
		return false;
	}

	function storeEvents(Array $events, $flags = OracleCalendar::IMPORT) {
		$ical = $this->_ical();
		foreach ($events as $event)
			$ical->appendChild($event);
		$mime = $this->_mime($ical);
		return $this->_connection->storeEvents($mime->toString(), $flags);
	}

	function deleteEvent($uid, $recurrenceid = null, $modifier = 0) {
		return $this->deleteEvents(array($uid), $recurrenceid, $modifier);
	}
	
	function deleteEvents($uids, $recurrenceid = null, $modifier = 0) {
		return $this->_connection->deleteEvents($uids, $recurrenceid, $modifier);
	}
	
	function storeTask(VCalendarObject $task, $flags = OracleCalendar::IMPORT) {
		if (is_array($ids = $this->storeTasks(array($task))))
			return reset($ids);
		return false;
	}

	function storeTasks(Array $tasks, $flags = OracleCalendar::IMPORT) {
		$ical = $this->_ical();
		foreach ($tasks as $task)
			$ical->appendChild($task);
		$mime = $this->_mime($ical);
		return $this->_connection->storeTasks($mime->toString(), $flags);
	}

	function deleteTask($uid) {
		return $this->deleteTasks(array($uid));
	}
	
	function deleteTasks($uids) {
		return $this->_connection->deleteTasks($uids);
	}
	
	function storeContact(VCalendarObject $contact, $flags = OracleCalendar::IMPORT) {
		if (is_array($ids = $this->storeContacts(array($contact))))
			return reset($ids);
		return false;
	}

	function storeContacts(Array $contacts, $flags = OracleCalendar::IMPORT) {
		$ical = $this->_ical();
		foreach ($contacts as $contact)
			$ical->appendChild($contact);
		return $this->_connection->storeContacts($ical->toString(), OracleCalendar::STREAM_NOT_MIME | $flags);
	}

	function deleteContact($uid) {
		return $this->deleteTasks(array($uid));
	}
	
	function deleteContacts($uids) {
		return $this->_connection->deleteTasks($uids);
	}

	function getContactsByQuery(OracleCalendarQuery $query, $flags = OracleCalendar::NONE, $properties = null) {
		return $this->_parse($this->_connection->getContactsByQuery($query, $flags, $properties));
	}

	function getContactsByUid($uids, $flags = OracleCalendar::NONE, $properties = null) {
		return $this->_parse($this->_connection->getContactsByUid($uids, $flags, $properties));
	}
	
	protected function _parse($response) {
		return VCalendar::parse(Mime::parse($response)->getContent())->getChildren();
	}

	protected function _ical() {
		$ical = new VCalendarObject('VCALENDAR');
		$ical->setProperty('VERSION', '2.0');
		$ical->setProperty('PRODID', '-//Synd (www.synd.info)//CSDK Extension 1.0//EN');
		return $ical;
	}
	
	protected function _mime($ical, $type = 'text/calendar') {
		$body = Mime::createTextpart($ical->toString(), '');
		$body->setHeader('Content-Type', "$type; charset=utf-8");
		$body->setHeader('Content-Transfer-Encoding', '8bit');

		$mime = Mime::createMultipart();
		$mime->addPart($body);
		$mime->setHeader('Content-Transfer-Encoding', '7bit');
		
		return $mime;
	}
}

/**
 * Adapts an Oracle Calendar Agenda to communicate using VCalendar objects
 */
abstract class AbstractOracleCalendarAgendaFacade implements IOracleCalendarAgenda {
	protected $_agenda = null;
	
	/**
	 * @param	IOracleCalendarAgenda	Agenda to wrap
	 */
	function __construct(IOracleCalendarAgenda $agenda) {
		$this->_agenda = $agenda;
	}
	
	function getEventsByRange($start, $end, $flags = OracleCalendar::NONE, Array $properties = array()) {
		return $this->_parse($this->_agenda->getEventsByRange(
			gmdate('Ymd\THis\Z', $start), gmdate('Ymd\THis\Z', $end), 
			$flags | OracleCalendarAgenda::COMBINED, $properties));
	}

	function getEventsByAlarmRange($start, $end, $flags = OracleCalendar::NONE, Array $properties = array()) {
		return $this->_parse($this->_agenda->getEventsByAlarmRange(
			gmdate('Ymd\THis\Z', $start), gmdate('Ymd\THis\Z', $end), 
			$flags | OracleCalendarAgenda::COMBINED, $properties));
	}

	function getTasksByRange($start, $end, $flags = OracleCalendar::NONE, Array $properties = array()) {
		return $this->_parse($this->_agenda->getTasksByRange(
			gmdate('Ymd\THis\Z', $start), gmdate('Ymd\THis\Z', $end), 
			$flags | OracleCalendarAgenda::COMBINED, $properties));
	}

	function getTasksByAlarmRange($start, $end, $flags = OracleCalendar::NONE, Array $properties = array()) {
		return $this->_parse($this->_agenda->getTasksByAlarmRange(
			gmdate('Ymd\THis\Z', $start), gmdate('Ymd\THis\Z', $end), 
			$flags | OracleCalendarAgenda::COMBINED, $properties));
	}

	protected function _parse($response) {
		return VCalendar::parse(Mime::parse($response)->getContent())->getChildren();
	}
}

/**
 * Adapts an OracleCalendarAgenda to communicate using VCalendar objects
 */
class OracleCalendarAgendaFacade extends AbstractOracleCalendarAgendaFacade {
	function getName() {
		return $this->_agenda->getName();
	}

	function getEmail() {
		return $this->_agenda->getEmail();
	}

	function getEventsByUid(Array $uids, $flags = OracleCalendar::NONE, Array $properties = array()) {
		return $this->_parse($this->_agenda->getEventsByUid($uids, $flags | OracleCalendarAgenda::COMBINED, $properties));
	}
	
	function getTasksByUid(Array $uids, $flags = OracleCalendar::NONE, Array $properties = array()) {
		return $this->_parse($this->_agenda->getTasksByUid($uids, $flags | OracleCalendarAgenda::COMBINED, $properties));
	}
}

/**
 * Adapts an OracleCalendarCompositeAgenda to communicate using VCalendar objects
 */
class OracleCalendarCompositeAgendaFacade extends AbstractOracleCalendarAgendaFacade {
	function getName() {
		foreach ($this->_agenda->getAgendas() as $agenda) {
			if (null != ($name = $agenda->getName()))
				return $name;
		}
		return null;
	}

	function getEmail() {
		foreach ($this->_agenda->getAgendas() as $agenda) {
			if (null != ($email = $agenda->getEmail()))
				return $email;
		}
		return null;
	}

	function getEventsByUid(Array $uids, $flags = OracleCalendar::NONE, Array $properties = array()) {
		$result = array();
		foreach ($this->getAgendas() as $agenda) {
			foreach ($agenda->getEventsByUid($uids, $flags, $properties) as $event) {
				if (null != ($uid = $event->getIdentifier()))
					$result[$uid] = $event;
				else
					$result[] = $event;
			}
		}
		return $result;
	}
	
	function getTasksByUid(Array $uids, $flags = OracleCalendar::NONE, Array $properties = array()) {
		$result = array();
		foreach ($this->getAgendas() as $agenda) {
			foreach ($agenda->getTasksByUid($uids, $flags, $properties) as $task) {
				if (null != ($uid = $task->getIdentifier()))
					$result[$uid] = $task;
				else
					$result[] = $task;
			}
		}
		return $result;
	}

	function addAgenda($agenda) {
		if ($agenda instanceof OracleCalendarAgendaFacade)
			return $this->_agenda->addAgenda($agenda->getAgenda());
		if ($agenda instanceof OracleCalendarCompositeAgendaFacade) {
			foreach ($agenda->getAgenda()->getAgendas() as $a)
				$this->_agenda->addAgenda($a);
			return true;
		}
		return $this->_agenda->addAgenda($agenda);
	}

	function getAgendas() {
		$result = array();
		foreach ($this->_agenda->getAgendas() as $agenda)
			$result[] = new OracleCalendarAgendaFacade($agenda);
		return $result;
	}
}

/** 
 * Adapts an OracleCalendarAgendaFacade to the CalendarAgenda interface
 */
abstract class AbstractOracleCalendarAgendaAdapter implements CalendarAgenda {
	protected $_agenda = null;
	
	function __construct(IOracleCalendarAgenda $agenda) {
		$this->_agenda = $agenda;
	}
	
	function getEventsByRange($start, $end) {
		return $this->_agenda->getEventsByRange($start, $end);
	}

	function getEventsByUid($uids) {
		return $this->_agenda->getEventsByUid($uids);
	}

	function addEvents($events) {
		return $this->_agenda->addEvents($events);
	}

	function addEvent($event) {
		return $this->_agenda->addEvent($event);
	}

	function newEvent($start = null, $end = null, $summary = null, $description = null) {
		$event = new VCalendarEvent($start, $end);
		$event->setProperty('SUMMARY', $summary);
		$event->setProperty('DESCRIPTION', $description);
		return $event;
	}
}

class OracleCalendarAgendaAdapter extends AbstractOracleCalendarAgendaAdapter {
	function newEvent($start = null, $end = null, $summary = null, $description = null) {
		$event = parent::newEvent($start, $end, $summary, $description);
		$event->addAttendee('mailto:'.$this->_agenda->getEmail());
		return $event;
	}
}

class OracleCalendarCompositeAgendaAdapter extends AbstractOracleCalendarAgendaAdapter {
	function newEvent($start = null, $end = null, $summary = null, $description = null) {
		$event = parent::newEvent($start, $end, $summary, $description);
		foreach ($this->_agenda->getAgendas() as $agenda)
			$event->addAttendee('mailto:'.$agenda->getEmail());
		return $event;
	}
}
