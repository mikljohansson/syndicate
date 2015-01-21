<?php
require_once 'core/Plugin.class.inc';
require_once 'core/lib/SyndLib.class.inc';
require_once 'core/lib/Template.class.inc';
require_once 'core/lib/calendar/VCalendar.class.inc';
require_once 'core/lib/calendar/Calendar.class.inc';

/**
 * Calendar issue plugin 
 * @see			OracleCalendar
 */
class synd_plugin_calendar extends Plugin {
	/**
	 * @access	private
	 */
	var $_calendar = null;
	
	/**
	 * @access	private
	 */
	var $_config = null;
	
	function __construct($config = null) {
		parent::__construct($config);
		SyndLib::attachHook('issue_view', array($this, '_callback_issue_view'));
		SyndLib::attachHook('issue_edit_pane', array($this, '_callback_issue_edit_pane'));
		SyndLib::attachHook('issue_data_validate', array($this, '_callback_issue_data_validate'));
		SyndLib::attachHook('issue_data_merge', array($this, '_callback_issue_data_merge'));
		SyndLib::attachHook('issue_onpreflush', array($this, '_callback_issue_onpreflush'));
	}
	
	/**
	 * Returns the calendar connection
	 * @return	Calendar
	 */
	function getCalendar() {
		global $synd_user;
		if (null === $this->_calendar) {
			$this->_calendar = CalendarManager::getConnection($this->_config);
			$this->_calendar->setIdentity($synd_user);
		}
		return $this->_calendar;
	}
	
	/**
	 * Returns selected users
	 * @param	synd_node_issue
	 * @param	array	Calendar screen POST variables (data[calendar])
	 * @return	array	List of User objects
	 */
	function getSelectedUsers($issue, $vars) {
		$client = $issue->getCustomer();
		if (!isset($vars['assigned']) || null === ($assigned = SyndNodeLib::getInstance($vars['assigned'])))
			$assigned = $issue->getAssigned();
		return array(
			$client->nodeId => $client, 
			$assigned->nodeId => $assigned);
	}
	
	/**
	 * @access	protected
	 */
	static function getEventKey($event) {
		return date('YmdHi', $event->getStart()).date('YmdHi', $event->getEnd());
	}
	
	function _view_deleteEvent(Request $request, Template $page) {
		if (!isset($request[0],$request[1]))
			throw new InvalidArgumentException();
		if (null === ($issue = SyndNodeLib::getInstance($request[0])))
			throw new NotFoundException();
		
		if (isset($issue->_calendar[$request[1]])) {
			unset($issue->_calendar[$request[1]]);
			$issue->_node_onchange_internal();
		}
		else if (isset($request[2])) {
			$eventids = (array)$issue->getAttribute('eventids');
			if (!isset($eventids[$request[2]]))
				throw new ForbiddenException();

			$calendar = $this->getCalendar();
			$calendar->deleteEvent($request[2]);
			
			unset($eventids[$request[2]]);
			$issue->setAttribute('eventids', $eventids);
		}
	}
	
	function _callback_issue_view(&$result, Template $page, $issue) {
		if (count($eventids = $issue->getAttribute('eventids')) && 
			count($users = SyndLib::getInstances(array_unique(call_user_func_array('array_merge',$eventids))))) {
			$calendar = $this->getCalendar();
			$agenda = $calendar->getCompositeAgenda($users);
			
			$template = clone $page;
			$template->assign('events', $agenda->getEventsByUid(array_keys($eventids)));
			$template->assign('eventids', $eventids);
			$result .= $template->fetch('plugin/calendar/events.tpl');
		}
	}
	
	function _callback_issue_edit_pane(&$result, $issue, $pane, $request) {
		if ($issue->isPermitted('admin')) {
			$template = DefaultTemplate::factory('plugin/calendar/pane_edit_calendar.tpl', $pane);
			$template->assign('plugin', $this);
			$template->assign('node', $issue);
			
			$pane->append('tabs', array(
				'uri' => tpl_link_jump('issue','edit',$issue->nodeId,'calendar'),
				'text' => SyndLib::translate('Calendar'),
				'template' => $template,
				'selected' => isset($request[0]) && 'calendar' == $request[0]));
		}
	}
	
	function _callback_issue_data_validate(&$result, $issue, $data) {
		if ($issue->isPermitted('admin')) {
			require_once 'core/lib/SyndDate.class.inc';
			if (!empty($data['calendar']['date']) && !SyndDate::checkdate($data['calendar']['date'])) {
				$result['calendar_date'] = SyndLib::translate("Invalid calendar date ('%s')", tpl_chop($data['calendar']['date'],10));
				if (!isset($data['calendar']['from']) || !SyndDate::checktime($data['calendar']['from']))
					$result['calendar_from'] = SyndLib::translate("Invalid calendar time ('%s')", tpl_chop($data['calendar']['from'],5));
				if (!isset($data['calendar']['to']) || !SyndDate::checktime($data['calendar']['to']))
					$result['calendar_to'] = SyndLib::translate("Invalid calendar time ('%s')", tpl_chop($data['calendar']['to'],5));
			}
		}
	}
	
	function _callback_issue_data_merge(&$result, $issue, $data) {
		if ($issue->isPermitted('admin') && !empty($data['calendar']['date'])) {
			$users = $this->getSelectedUsers($issue, $data['calendar']);
			$calendar = $this->getCalendar();
			$agenda = $calendar->getCompositeAgenda($users);

			$title = tpl_chop($issue->getTitle(),75).' #'.$issue->objectId();
			$description  = tpl_chop($issue->getDescription(),750);
			$description .= "\r\n\r\n".tpl_request_host().tpl_view($issue->getHandler(),$issue->objectId());

			$event = $agenda->newEvent(
				strtotime($data['calendar']['date'].' '.$data['calendar']['from']),
				strtotime($data['calendar']['date'].' '.$data['calendar']['to']),
				$title, $description);
			$event->_users = SyndLib::invoke($users,'id');

			$issue->_calendar = array();
			$issue->_calendar[self::getEventKey($event)] = $event;
			$issue->_node_onchange_internal();
		}
	}
	
	function _callback_issue_onpreflush(&$result, $issue) {
		if (isset($issue->_calendar)) {
			$calendar = $this->getCalendar();

			$eventids = (array)$issue->getAttribute('eventids');
			foreach (array_keys($issue->_calendar) as $key) {
				if (false !== ($id = $calendar->addEvent($issue->_calendar[$key])))
					$eventids[$id] = $issue->_calendar[$key]->_users;
			}
			
			$issue->setAttribute('eventids', $eventids);
			unset($issue->_calendar);
			$issue->_node_onchange_internal();
		}
	}
}

/**
 * Adapts a VCalendarEvent to the CalendarEvent interface
 */
class CalendarEventAdapter implements CalendarEvent {
	private $_issue = null;
	private $_event = null;
	private $_attributes = null;
	private $_writable = null;
	private $_page = null;
	
	/**
	 * @param	synd_node_issue		The issue 
	 * @param	CalendarEvent		Event to wrap
	 * @param	string						Additional attributes
	 * @param	bool						Render this event writable
	 */
	function __construct(Template $page, $issue, $event, $attributes = array('class' => 'Event'), $writable = false) {
		$this->_page = $page;
		$this->_issue = $issue;
		$this->_event = $event;
		$this->_attributes = $attributes;
		$this->_writable = $writable;
	}
	
	function toString() {
		$template = clone $this->_page;
		$template->assign('issue', $this->_issue);
		$template->assign('event', $this->_event);
		$template->assign('writable', $this->_writable);
		$template->assign('key', synd_plugin_calendar::getEventKey($this->_event));
		$template->assign('id', $this->_event->getIdentifier());
		return $template->fetch(tpl_design_path('plugin/calendar/event.tpl'));
	}
	
	function getAttributes() {
		return $this->_attributes;
	}
	
	function getIdentifier() {
		return $this->_event->getIdentifier();
	}

	function getSummary() {
		return $this->_event->getSummary();
	}

	function getDescription() {
		return $this->_event->getDescription();
	}

	function addAttendee($attendee) {
		$this->_event->addAttendee($attendee);
	}

	function getAttendees() {
		return $this->_event->getAttendees();
	}

	function getStart() {
		return $this->_event->getStart();
	}
	
	function getEnd() {
		return $this->_event->getEnd();
	}
}
