<?php
require_once 'core/lib/Mime.class.inc';

/** 
 * iCalendar parser
 * @access		public
 * @package		synd.core.model
 */
class VCalendar {
	const QUOTED_PRINTABLE = 1;
	const UTF8 = 2;
	
	/**
	 * Parses a vCalendar message
	 *
	 * Returns null if the message could not be parsed
	 *
	 * @param	string	Message buffer
	 * @return	VCalendarObject
	 */
	static function parse($buffer) {
		$buffer = preg_replace('/\r?\n[ \t]|=\r?\n/', '', $buffer);
		$stream = preg_split('/[\r\n]+/', trim($buffer));
		
		$stack = array();
		$top = null;
		
		while (null !== ($line = array_shift($stream))) {
			$parts = explode(':', trim($line), 2);
			if (2 != count($parts))
				continue;
			
			$parameters = explode(';', $parts[0]);
			$property = array_shift($parameters);
			$flags = 0;
			
			if (in_array('ENCODING=QUOTED-PRINTABLE', array_map('strtoupper', $parameters))) {
				$parts[1] = quoted_printable_decode($parts[1]);
				$flags |= VCalendar::QUOTED_PRINTABLE;
			}
			if (in_array('CHARSET=UTF-8', array_map('strtoupper', $parameters))) {
				$parts[1] = Mime::charset($parts[1], 'utf-8');
				$flags |= VCalendar::UTF8;
			}
			
			$parts[1] = VCalendar::decode($parts[1]);
			
			if ('BEGIN' == $property) {
				switch (strtoupper($parts[1])) {
					case 'VEVENT':
						$next = new VCalendarEvent();
						break;
					case 'VTODO':
						$next = new VCalendarTodo();
						break;
					case 'VJOURNAL':
						$next = new VCalendarJournal();
						break;
					case 'VNOTE':
						$next = new VCalendarNote();
						break;
					default:
						$next = new VCalendarObject($parts[1]);
				}
				
				// Push child object
				if (!empty($stack))
					end($stack)->appendChild($next);
				if (null === $top)
					$top = $next;
				$stack[] = $next;
			}
			else if (!empty($stack)) {
				if ('END' != $property) 
					end($stack)->setProperty($property, $parts[1], $flags);
				else if (end($stack)->getName() == $parts[1])
					array_pop($stack);
			}
		}

		return $top;
	}

	public static function encode($value) {
		return preg_replace('/[\\\\;:,]/', '\\\\\\0', $value);
	}
	
	public static function decode($value) {
		return preg_replace('/\\\\([\\\\;:,])/', '\1', $value);
	}
}

/** 
 * iCalendar component
 * @access		public
 * @package		synd.core.model
 */
class VCalendarObject {
	protected $_name = null;
	protected $_children = array();
	protected $_properties = array();
	
	/**
	 * @param	string	Name of object {VCALENDAR, VEVENT, VTODO, ...}
	 */
	function __construct($name = 'VCALENDAR') {
		$this->_name = $name;
	}
	
	function getName() {
		return $this->_name;
	}
	
	function toString() {
		$string = "BEGIN:{$this->_name}\r\n";
		
		foreach ($this->_properties as $property) {
			$value = VCalendar::encode($property[1]);
			
			if ($property[2] & VCalendar::UTF8 || preg_match('/[\x80-\xFF]/', $value)) {
				$property[0] .= ';CHARSET=UTF-8';
				$value = Mime::charset($value, null, 'utf-8');
			}
			if ($property[2] & VCalendar::QUOTED_PRINTABLE || preg_match('/[\x00-\x1F\x7F-\xFF]/', $value)) {
				$property[0] .= ';ENCODING=QUOTED-PRINTABLE';
				$value = Mime::quotedPrintableEncode($value, 76, 76-strlen($property[0])-1);
			}
			$string .= $property[0].':'.$value."\r\n";
		}
		
		foreach (array_keys($children = $this->getChildren()) as $key)
			$string .= $children[$key]->toString();
		
		$string .= "END:{$this->_name}\r\n";
		return preg_replace('/[^\r\n]{74}(?![\r\n])/', "\\0\r\n ", $string);
	}
	
	function appendChild($child) {
		return $this->_children[] = $child;
	}
	
	function getChildren() {
		return $this->_children;
	}
	
	function toTimestamp($time) {
		return strtotime(preg_replace('/^(\d{4})(\d\d)(\d\d)T(\d\d)(\d\d)(\d\d)/', '\1-\2-\3 \4:\5:\6', $time));
	}
	
	/**
	 * Append a new property
	 * @param	string	Name of property
	 * @param	string	Value of property
	 * @param	integer	Property attributes {VCalendar::QUOTED_PRINTABLE, VCalendar::UTF8, ...}
	 */
	function setProperty($name, $value, $flags = 0) {
		$this->_properties[] = array($name, (string)$value, $flags);
	}
	
	/**
	 * Replaces the first instances of a property
	 * @param	string	Name of property
	 * @param	string	Value of property
	 * @param	integer	Property attributes {VCalendar::QUOTED_PRINTABLE, VCalendar::UTF8, ...}
	 */
	function replaceProperty($name, $value, $flags = 0) {
		foreach ($this->_properties as $key => $property) {
			if (strtoupper($name) == strtoupper($property[0])) {
				$this->_properties[$key] = array($name, (string)$value, $flags);
				break;
			}
		}
	}
	
	/**
	 * Returns the value of the first mathing property
	 * @param	string	Property name
	 * @return	string	Returns null if nothing found
	 */
	function getProperty($name) {
		foreach ($this->_properties as $property) {
			if (strtoupper($name) == strtoupper($property[0]))
				return $property[1];
		}
		return null;
	}

	function setDateProperty($name, $ts) {
		$this->setProperty($name, gmdate('Ymd\THis\Z', $ts));
	}

	function getDateProperty($name) {
		if (null != ($value = $this->getProperty($name)))
			return $this->toTimestamp($value);
		return null;
	}

	/**
	 * START can not occur after DUE
	 */
	function setStart($ts) {
		if (null != ($due = $this->getDue()) && $ts > $due)
			$ts = strtotime('-1 days', $due);
		$this->setProperty('DTSTART', gmdate('Ymd\THis\Z', $ts));
	}
	
	function getStart() {
		if (null != ($start = $this->getProperty('DTSTART')))
			return $this->toTimestamp($start);
		return null;
	}
	
	/**
	 * DUE can not occur prior to START
	 */
	function setDue($ts) {
		if (null != ($start = $this->getStart()) && $ts < $start)
			$ts = strtotime('+1 days', $start);
		$this->setProperty('DUE', gmdate('Ymd\THis\Z', $ts));
	}

	function getDue() {
		if (null != ($due = $this->getProperty('DUE')))
			return $this->toTimestamp($due);
		return null;
	}

	/**
	 * DTEND can not occur prior to START
	 */
	function setEnd($ts) {
		if (null != ($start = $this->getStart()) && $ts < $start)
			$ts = strtotime('+1 days', $start);
		$this->setProperty('DTEND', gmdate('Ymd\THis\Z', $ts));
	}

	function getEnd() {
		if (null != ($end = $this->getProperty('DTEND')))
			return $this->toTimestamp($end);
		return null;
	}

	function setCompleted($ts) {
		$ts = strtotime('08:00:00', $ts);
		$this->setProperty('COMPLETED', gmdate('Ymd\THis\Z', $ts));
	}

	function getCompleted() {
		if (null != ($completed = $this->getProperty('COMPLETED')))
			return $this->toTimestamp($completed);
		return null;
	}

	function setAlarm($ts, $subject) {
		$this->setProperty('AALARM', gmdate('Ymd\THis\Z', $ts).';;;'.substr($subject,0,20));
		$this->setProperty('DALARM', gmdate('Ymd\THis\Z', $ts).';;;'.substr($subject,0,20));
	}

	function getSummary() {
		return $this->getProperty('SUMMARY');
	}
	
	/**
	 * A short summary or title without linebreaks
	 * @param	string
	 */
	function setSummary($summary) {
		$this->setProperty('SUMMARY', $summary);
	}

	function getDescription() {
		return $this->getProperty('DESCRIPTION');
	}
	
	/**
	 * A longer description or text
	 * @param	string
	 */
	function setDescription($description) {
		$this->setProperty('DESCRIPTION', $description);
	}

	function getLocation() {
		return $this->getProperty('LOCATION');
	}
	
	function setLocation($location) {
		$this->setProperty('LOCATION', $location);
	}

	function setComment($comment) {
		$this->setProperty('COMMENT', $comment);
	}
}

/**
 * Calendar event (VEVENT)
 */
class VCalendarEvent extends VCalendarObject /* implements CalendarEvent */ {
	/**
	 * @param	integer	Start time as a unix timestamp
	 * @param	integer	Due time as a unix timestamp
	 * @param	string	Short title
	 * @param	string	Longer description
	 */
	function __construct($start = null, $end = null, $summary = null, $description = null) {
		parent::__construct('VEVENT');
		if (null != $start)
			$this->setStart($start);
		if (null != $end)
			$this->setEnd($end);
		if (null != $summary)
			$this->setSummary($summary);
		if (null != $description)
			$this->setDescription($description);
	}
	
	/**
	 * Returns the 'UID' property
	 * @return	string
	 */
	function getIdentifier() {
		return $this->getProperty('UID');
	}
	
	function addAttendee($attendee) {
		$this->setProperty('ATTENDEE', $attendee);
	}
	
	function getAttendees() {
		$result = array();
		foreach ($this->_properties as $property) {
			if ('ATTENDEE' == strtoupper($property[0]))
				$result[] = $property[1];
		}
		return $result;
	}
}

/**
 * Calendar todo task (VTODO)
 */
class VCalendarTodo extends VCalendarObject {
	/**
	 * @param	integer	Start time as a unix timestamp
	 * @param	integer	Due time as a unix timestamp
	 * @param	string	Short title
	 * @param	string	Longer description
	 */
	function __construct($start = null, $due = null, $summary = null, $description = null) {
		parent::__construct('VTODO');
		if (null != $start)
			$this->setStart($start);
		if (null != $due)
			$this->setDue($due);
		if (null != $summary)
			$this->setSummary($summary);
		if (null != $description)
			$this->setDescription($description);
	}
}

/**
 * Calendar note (VJOURNAL)
 */
class VCalendarJournal extends VCalendarObject {
	/**
	 * @param	string	Short title
	 * @param	string	Longer description
	 */
	function __construct($summary = null, $description = null) {
		parent::__construct('VJOURNAL');
		if (null != $summary)
			$this->setSummary($summary);
		if (null != $description)
			$this->setDescription($description);
	}
}

/**
 * Calendar note (VNOTE)
 */
class VCalendarNote extends VCalendarObject {
	/**
	 * @param	string	Longer description
	 */
	function __construct($description = null) {
		parent::__construct('VNOTE');
		if (null != $description)
			$this->setDescription($description);
	}

	/**
	 * A longer description or text
	 * @param	string
	 */
	function setDescription($description) {
		$this->setProperty('BODY', $description);
	}
}
