<?php
require_once 'core/model/DatabaseEntity.class.inc';
require_once 'core/lib/auth/User.class.inc';

/**
 * A cost center is a billable organizational unit
 *
 * Leases and projects can have a cost center take the care of the 
 * associated costs and invoices instead of the client.
 */
class synd_node_costcenter extends AbstractDatabaseEntity implements User {
	function __factory() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_costcenter');
		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_CREATE'] = time();
		parent::__factory();
	}

	function __clone() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_costcenter');
		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_CREATE'] = time();
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_costcenter');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}
	
	function hasPermission($user, $perm, $arg = null) {
		if ('merge' == $perm && in_array($arg, array(
			'PARENT_NODE_ID','INFO_NUMBER','INFO_PROJECT_CODE','INFO_HEAD','INFO_DESC','INFO_LIABLE',
			'INFO_STREET','INFO_ZIP','INFO_CITY','INFO_COUNTRY','INFO_EMAIL','INFO_PHONE','INFO_FAX')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}

	function getHandler() {
		return 'inventory';
	}

	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function setParent($parent) {
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
	}
	
	function toString() {
		return $this->data['INFO_HEAD'];
	}
	
	function getDescription() {
		return $this->data['INFO_DESC'];
	}
	
	static function resolveLogin($username) {
		return null;
	}
	
	static function resolveEmail($email) {
		return null;
	}

	function validatePassword($password) {
		return false;
	}

	function getLogin() {
		return $this->getNumber();
	}

	function getEmail() {
		return $this->data['INFO_EMAIL'];
	}
	
	function getPhone() {
		return $this->data['INFO_PHONE'];
	}
	
	function getContact() {
		return $this->getLogin();
	}
	
	function getAddress() {
		return array_filter(array(
			'STREET'	=> $this->data['INFO_STREET'],
			'ZIP'		=> $this->data['INFO_ZIP'],
			'CITY'		=> $this->data['INFO_CITY'],
			'COUNTRY'	=> $this->data['INFO_COUNTRY'],
			));
	}

	function getSocialSecurityNumber() {
		return $this->getNumber();
	}
	
	function getNumber() {
		return $this->data['INFO_NUMBER'];
	}

	function getProjectCode() {
		return $this->data['INFO_PROJECT_CODE'];
	}
	
	/**
	 * Returns the user primarily responsible for this cost center
	 * @return	User
	 */
	function getLiableClient() {
		return SyndNodeLib::getInstance('user_case.'.$this->data['INFO_LIABLE']);
	}
	
	function getOrganization() {
		return SyndNodeLib::getInstance('null.null');
	}

	function getRoles() {
		return array();
	}
	
	function getIdentifier() {
		return $this->getLogin();
	}
	
	static function findIdentifier($id) {
		return self::resolveEmail($id);
	}
	
	static function findInstances($query) {
		return SyndType::factory('null_collection');
	}

	function getChildren() {
		return array();
	}

	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();
		$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_UPDATE'] = time();
	}
}
