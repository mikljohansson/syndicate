<?php

/**
 * Abstract calendar factory
 */
abstract class CalendarManager {
	protected static $_instances = array();

	/**
	 * Calendar connection factory
	 *
	 * If the DSN is an array the first element must be the actual
	 * DSN to be used. For example
	 *
	 * <code>
	 * $calendar = CalendarConnection::factory(array(
	 *   'ocal://user:password@calendar.example.com',
	 *   'ldap3://calendar.example.com/o=example.com',
	 *   false));
	 * </code>
	 *
	 * @param	string		Calendar connection string
	 * @return	Calendar
	 */
	static function getConnection($dsn) {
		if (isset(self::$_instances[$dsn]))
			return self::$_instances[$dsn];
		$driver = false !== ($i = strpos($dsn, ':')) ? substr($dsn, 0, $i) : $dsn;
		$class = 'synd_calendar_'.$driver;
		require_once 'core/lib/calendar/driver/'.$driver.'.class.inc';
		return self::$_instances[$dsn] = call_user_func(array($class, 'factory'), $class, $dsn);
	}
}

/**
 * Calendar connection interface
 * @access		public
 * @package		synd.core.lib
 */
interface Calendar {
	/**
	 * Calendar connection factory
	 * @param	string	Driver class name
	 * @param	string	DSN
	 * @return	Calendar
	 */
	static function factory($driver, $dsn);
	
	/**
	 * Sets the current user
	 *
	 * This will most likely require some sort of SYSOP privileges
	 * for the supplied user on the Calendar server.
	 *
	 * @param	User	The user to run as
	 */
	function setIdentity($user);

	/**
	 * Open an agenda
	 * @param	User	User whos agenda to open
	 * @return	CalendarAgenda
	 */
	function getAgenda($user);

	/**
	 * Returns a new composite agenda
	 * @param	array	Array of User's
	 * @return	CalendarAgenda
	 */
	function getCompositeAgenda($users);

	/**
	 * Adds or replaces an event
	 * @param	VCalendarEvent	Event object to store
	 * @return	string	Returns the event id, false of failure
	 */
	function addEvent($event);

	/**
	 * Adds or replaces a list of events
	 * @param	array	Array of VCalendarEvent to store
	 * @return	array	Returns an array of event ids, false on failure
	 */
	function addEvents($events);

	/**
	 * Delete an event
	 * @param	string	UID of event to delete
	 * @return	bool
	 */
	function deleteEvent($id);
	
	/**
	 * Delete an list of events
	 * @param	string	UID of event to delete
	 * @return	bool
	 */
	function deleteEvents($ids);
}

/**
 * Calendar agenda interface
 * @access		public
 * @package		synd.core.lib
 */
interface CalendarAgenda {
	/**
	 * Returns events given a DTSTART range
	 * @param	integer	Unix timestamp of range start
	 * @param	integer	Unix timestamp of range end
	 * @return	array
	 */
	function getEventsByRange($start, $end);

	/**
	 * Returns events given a list of UID
	 * @param	array	List of UID strings
	 * @return	array
	 */
	function getEventsByUid($uids);

	/**
	 * Adds or replaces an list events in this agenda
	 * @see		newEvent()
	 * @param	array	List of ICalEvents to store
	 * @return	array	Returns list of event UIDs, false on failure
	 */
	function addEvents($events);

	/**
	 * Adds or replaces an event in this agenda
	 * @see		newEvent()
	 * @param	VCalendarEvent	Event object to store
	 * @return	string	Returns the event UID, false on failure
	 */
	function addEvent($event);
	
	/**
	 * Creates a new event to be stored into this agenda
	 * @see		addEvent()
	 * @param	integer	Start time as a unix timestamp
	 * @param	integer	Due time as a unix timestamp
	 * @param	string	Short title
	 * @param	string	Longer description
	 */
	function newEvent($start = null, $end = null, $summary = null, $description = null);
}

/**
 * Calendar event (typically a meeting) interface
 * @access		public
 * @package		synd.core.model
 */
interface CalendarEvent {
	/**
	 * Returns the unique id of this event
	 * @return	string
	 */
	function getIdentifier();

	/**
	 * Returns a short title
	 * @return	string
	 */
	function getSummary();

	/**
	 * Returns a longer description
	 * @return	string
	 */
	function getDescription();

	/**
	 * Adds an attendee to this event
	 * @see		synd_node_user_case
	 * @param	string	Attendee email/login/..
	 */
	function addAttendee($attendee);

	/**
	 * Returns event attendes
	 * @return	array	Attendee email/login/.. as strings
	 */
	function getAttendees();

	/**
	 * Returns the timestamp this event begins
	 * @return	integer
	 */
	function getStart();
	
	/**
	 * Returns the timestamp this event ends
	 * @return	integer
	 */
	function getEnd();
}
