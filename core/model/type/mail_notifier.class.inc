<?php

/**
 * Issue mailnotification helper.
 */
class synd_type_mail_notifier extends SyndType {
	/**
	 * The registered listeners. 
	 * array('onevent' => array('node.user.123'))
	 * @access	private
	 */
	var $_notify = null;

	/**
	 * Temporary listeners.
	 * @access	private
	 */
	var $_temporary = null;

	/**
	 * Temporary listener addresses.
	 * @access	private
	 */
	var $_tempAddresses = null;

	/**
	 * Events triggered in this session.
	 * @access	private
	 */
	var $_active = null;

	/**
	 * @access	private
	 */
	var $_attachments = null;

	/**
         * Don't serialize the temporary variables
         */
        function __sleep() {
                $vars = array_keys(get_object_vars($this));
                if (false !== ($key = array_search('_temporary', $vars)))
                        unset($vars[$key]);
                if (false !== ($key = array_search('_tempAddresses', $vars)))
                        unset($vars[$key]);
                if (false !== ($key = array_search('_active', $vars)))
                        unset($vars[$key]);
                if (false !== ($key = array_search('_attachments', $vars)))
                        unset($vars[$key]);
                return $vars;
        }

	/**
	 * Add an email to an event.
	 * @param	string	Event to attach to
	 * @param	User	User to register for event
	 */
	function addListener($event, $listener) {
		if (!$listener->isNull()) {
			$this->_notify[$event][] = $listener->id();
			$this->_notify[$event] = array_unique($this->_notify[$event]);
		}
	}
	
	/**
	 * Removes an email to an event.
	 * @param	string	Event to remove from
	 * @param	User	User to unregister from event
	 */
	function removeListener($event, $listener) {
		if (isset($this->_notify[$event]) && 
			false !== ($key = array_search($listener->id(), $this->_notify[$event])))
			unset($this->_notify[$event][$key]);
		if (empty($this->_notify[$event]))
			unset($this->_notify[$event]);
	}
	
	/**
	 * Add a temporary listener email to all events
	 * @param	User	User to register
	 */
	function addTemporary($listener) {
		assert('is_object($listener)');
		if (!$listener->isNull() && !in_array($listener->id(), (array)$this->_temporary))
			$this->_temporary[] = $listener->id();
	}
	
	/**
	 * Add a temporary raw email address as listener to all events
	 * @param	string	Email address
	 */
	function addTemporaryEmail($address) {
		$this->_tempAddresses[] = $address;
	}
	
	function getTemporaryUsers() {
		return SyndLib::getInstances($this->_temporary);
	}
	
	function getTemporaryEmails() {
		return $this->_tempAddresses;
	}

	/**
	 * Adds an attachment to outgoing emails
	 * @param	synd_type_file	File to attach
	 */
	function addAttachment($attachment) {
		$this->_attachments[] = $attachment;
	}
	
	/**
	 * Handle an event
	 * @param	synd_type_event	Event to handle
	 */
	function handleEvent($event) {
		$this->_active[] = $event;
	}
	
	/**
	 * Returns the list of listeners registered for an event
	 * @param	string	Event to check
	 * @return	array	Array of User objects
	 */
	function getListeners($event) {
		if (!isset($this->_notify[$event]))
			$listeners = SyndLib::getInstances((array)$this->_temporary);
		else {
			$listeners = SyndLib::array_merge_assoc(
				SyndLib::getInstances($this->_notify[$event]), 
				SyndLib::getInstances((array)$this->_temporary));
		}
		return SyndLib::array_kintersect($listeners, array_unique(
			SyndLib::invoke(array_filter($listeners, 'is_object'), 'getEmail')));
	}
	
	/**
	 * Returns the registered events for a listener
	 * @param	User
	 * @return	array
	 */
	function getListenerEvents($listener) {
		$events = array();
		foreach ((array)$this->_notify as $event => $ids) {
			if (false !== array_search($listener->id(), $ids))
				$events[] = $event;
		}
		return $events;
	}

	/**
	 * Returns true if a listener is registered on an event
	 * @param	string	Event to check
	 * @param	User
	 * @return	bool
	 */
	function isRegistered($event, $listener) {
		return in_array($listener->id(), (array)$this->_temporary) ||
			false !== array_search($event, $this->getListenerEvents($listener));		
	}
	
	/**
	 * Returns true if a listener is permanently registered on an event
	 * @param	string	Event to check
	 * @param	User
	 * @return	bool
	 */
	function isRegisteredPermanent($event, $listener) {
		return false !== array_search($event, $this->getListenerEvents($listener));		
	}

	function getRegisteredEvents() {
		return array_keys((array)$this->_notify);
	}
	
	/**
	 * Returns sorted list of triggered events. Highest priority first.
	 * @return	array	array(object synd_type_event, ...)
	 */
	function getActiveEvents() {
		// The array_filter is to remove artifacts from the prev datamodel
		return array_reverse(SyndLib::sort(array_filter((array)$this->_active,'is_object'), 'getPrecedence'));
	}

	/**
	 * Returns list of active events and listener ids
	 * @return	array	array(event instance id => array(user instance id, ...))
	 */
	function getActiveListeners() {
		$result = array();
		$seenListeners = array();
		foreach ($this->getActiveEvents() as $event) {
			foreach (array_keys($listeners = $this->getListeners($event->classId())) as $key2) {
				if (!in_array($listeners[$key2]->getEmail(), $seenListeners)) {
					$result[$event->id()][] = $listeners[$key2]->id();
					$seenListeners[] = $listeners[$key2]->getEmail();
				}
			}
		}

		foreach ((array)$this->_tempAddresses as $email) {
			if (!in_array($email, $seenListeners)) {
				$result[$event->id()][] = "node.user_case.$email";
				$seenListeners[] = $email;
			}
		}
		
		return $result;
	}

	/**
	 * Sends any queued emails
	 */
	function send() {
		global $synd_user;
		require_once 'core/lib/TemplateLib.inc';
		require_once 'core/lib/Mime.class.inc';
		
		$seen = array();
		foreach ($this->getActiveEvents() as $event) {
			$users = $this->getListeners($event->classId());
			$receivers = array_diff(array_merge(SyndLib::invoke($users, 'getEmail'), (array)$this->_tempAddresses), $seen);
			
			if (!empty($receivers)) {
				$seen = array_merge($seen, $receivers);
				$mime = Mime::createTextpart($event->getEmailBody($users));

				// Attach specified files
				if (!empty($this->_attachments)) {
					$mime = Mime::createMultipart(array($mime));
					foreach (array_keys($this->_attachments) as $key2) {
						$mime->addPart(Mime::createTextpart(
							file_get_contents($this->_attachments[$key2]->path()), 
							$this->_attachments[$key2]->toString()));
					}
				}

				if (count($senders = $event->getEmailSenders()) > 1) 
					$mime->setHeader('Sender', reset($senders));
				if (count($senders))
					$mime->setHeader('From', implode(', ', $senders));
				if (null == ($subject = $event->getEmailSubject()))
					 continue;

				// Primary To: address and Cc: any other receivers
				$to = array_shift($receivers);
				if (!empty($receivers))
					$mime->setHeader('Cc', implode(', ', $receivers));

				$mime->setHeader('Precedence', 'bulk');

				// Send email
				$headers = array();
				foreach ($mime->getMessageHeaders() as $name => $value)
					$headers[] = $name.': '.MimeHeader::encodeHeader($name, $value);

				$flags = null != ($envelope = $event->getEnvelopeAddress()) ? "-f $envelope" : null;
				mail($to, MimeHeader::encodeHeader('Subject', $subject), $mime->getMessageBody(), implode("\r\n", $headers), $flags);
			}
		}

		$this->_temporary = null;
		$this->_tempAddresses = null;
		$this->_active = null;
		$this->_attachments = null;
	}
}
