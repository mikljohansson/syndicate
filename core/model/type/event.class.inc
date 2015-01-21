<?php

class synd_type_event extends SyndType /* implements IDomainEvent */ {
	var $_user = null;
	var $_ts = null;

	function __construct($parent){
		global $synd_user;
		parent::__construct();
		$this->setParent($parent);
		$this->_ts = time();
		$this->_user = $synd_user->nodeId;
	}
	
	function toString() {
		return $this->classId();
	}
	
	function isAuthorized($user) {
		$parent = $this->getParent();
		return $parent->hasPermission($user, 'read');
	}
	
	function getUser() {
		if (null === ($user = SyndNodeLib::getInstance($this->_user)))
			$user = SyndNodeLib::getInstance('user_null.null');
		return $user;
	}
	
	function getTime() {
		return $this->_ts;
	}
	
	function getTimestamp() {
		return $this->_ts;
	}
	
	function getPrecedence() {
		return 0;
	}

	function getEmailSenders() {
		if (null !== ($parent = $this->getParent()) && !$parent->isNull())
			return $parent->getEmailSenders();
		return null;
	}
	
	function getEnvelopeAddress() {
		if (null !== ($parent = $this->getParent()) && !$parent->isNull())
			return $parent->getEnvelopeAddress();
		return null;
	}

	function getEmailSubject() {
		if (null !== ($parent = $this->getParent()) && !$parent->isNull())
			return $parent->getEmailSubject();
		return null;
	}

	function getEmailBody($users) {
		if (null !== ($parent = $this->getParent()) && !$parent->isNull())
			return $parent->getEmailBody($this, $users);
		return null;
	}
}
