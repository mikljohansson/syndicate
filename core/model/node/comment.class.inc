<?php
require_once 'core/model/DatabaseEntity.class.inc';
require_once 'core/lib/TemplateLib.inc';

class synd_node_comment extends AbstractDatabaseEntity {
	function __factory() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_comment');
		$this->data['CLIENT_NODE_ID'] = $synd_user->nodeId;
		$this->data['CLIENT_HOST'] = $_SERVER['REMOTE_ADDR'];
		$this->data['TS_CREATE'] = time();
		parent::__factory();
	}

	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_comment');
		parent::__clone();
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_comment');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}
	
	function hasPermission($user, $perm, $arg = null) {
		$parent = $this->getParent();
		if ('write' == $perm && $this->isNew() && $parent->hasPermission($user, 'read'))
			return true;
		if ('merge' == $perm && in_array($arg, array('CLIENT_NAME','INFO_BODY')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}

	function getIndexDocument($index) {
		return $index->createFragment($this->data['INFO_BODY']);
	}

	function setParent($parent) {
		parent::setParent($parent);
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
		$this->_save();
	}

	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function getCustomer() {
		if (null === ($client = SyndNodeLib::getInstance($this->data['CLIENT_NODE_ID'])))
			$client = SyndNodeLib::getInstance('user_case.'.$this->data['CLIENT_NAME']);
		return $client;
	}

	function getBody() {
		return preg_replace('/\s*[\n\r]\s*/', ' ', $this->data['INFO_BODY']);
	}
	
	function getDescription() {
		return $this->data['INFO_BODY'];
	}
	
	function getCreatedTimestamp() {
		return $this->data['TS_CREATE'];
	}
	
	function setContent($content) {
		$this->data['INFO_BODY'] = $content;
		$this->_save();
	}

	function validate($data) {
		global $synd_user;
		$errors = array();
		$parent = SyndNodeLib::getInstance($data['PARENT_NODE_ID']);
		if (null == $parent || !$parent->isPermitted('append', $this))
			$errors['PARENT_NODE_ID'] = SyndLib::translate('Invalid article selected');
		if (!$synd_user->isNull())
			$data['CLIENT_NAME'] = $synd_user->toString();
		return array_merge(parent::validate($data), $errors);
	}

	function merge($data) {
		if (isset($data['CLIENT_NAME']))
			$data['CLIENT_NAME'] = strip_tags($data['CLIENT_NAME']);
		if (isset($data['INFO_BODY']))
			$data['INFO_BODY'] = strip_tags($data['INFO_BODY']);
		parent::merge($data);
	}
}
