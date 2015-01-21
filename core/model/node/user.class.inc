<?php
require_once 'core/model/node/node.class.inc';
require_once 'core/lib/auth/User.class.inc';

class synd_node_user extends synd_node_node implements User {
	function __construct($storage, $database, $data) {
		if (null != $data['INFO_PHOTO'])
			$data['INFO_PHOTO'] = unserialize($data['INFO_PHOTO']);
		parent::__construct($storage, $database, $data);
	}
	
	function __factory() {
		parent::__factory();
		$this->data['TS_LOGIN'] = time();
	}

	static function getEntityQuery(DomainStorage $storage) {
		$query = parent::getEntityQuery($storage);
		$query->join('synd_node_user');
		return $query;
	}
	
	function isInheritedFrom($interface) {
		if ('User' == $interface)
			return true;
		return parent::isInheritedFrom($interface);
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('read' == $perm && in_array($arg, array(null,
			'full_view_blog.tpl','full_view_media.tpl','full_view_texts.tpl',
			'full_view_comments.tpl','full_edit_step1.tpl','full_edit_step2.tpl',
			'full_edit_password.tpl')))
			return true;
		if ('write' == $perm && $this->isNew() && null != ($module = Module::getInstance('user')) && 
			$module->isPermitted('signup'))
			return true;
		if ('write' == $perm && $this->hasAuthorization($user, $this))
			return true;
		if ('append' == $perm && ($arg instanceof synd_node_comment))
			return true;
		if ('merge' == $perm && in_array($arg, array(
			'USERNAME','PASSWORD','INFO_EMAIL','INFO_PHOTO','INFO_HEAD','INFO_DESC')))
			return true;
		return parent::hasPermission($user, $perm, $arg);
	}

	function isValid() {
		return null != $this->data['USERNAME'] && null != $this->data['PASSWORD'];
	}
	

	function toString() {
		return null != $this->data['INFO_HEAD'] ? $this->data['INFO_HEAD'] : $this->getLogin();
	}

	function __toString() {
		return $this->toString().' ('.$this->getContact().')';
	}

	function setUsername($username) {
		$this->data['USERNAME'] = $username;
		$this->_save();
	}
	
	function setPassword($password) {
		$this->data['PASSWORD'] = md5($password);
		$this->_save();
	}

	function getHandler() {
		return 'user';
	}

	function getIndexDocument($index) {
		return $index->createFragment($this->data['INFO_HEAD'].' '.$this->data['USERNAME'],' '.$this->data['INFO_DESC']);
	}

	function getThumb($x = null, $y = null) {
		if (null != $this->data['INFO_PHOTO'])
			return $this->data['INFO_PHOTO']->getResizedUri($x, $y);
		return null;
	}

	function getLogin() {
		return $this->data['USERNAME'];
	}

	function getContact() {
		return $this->getLogin();
	}
	
	function getPhone() {
		return null;
	}

	function getEmail() {
		return $this->data['INFO_EMAIL'];
	}

	function getSocialSecurityNumber() {
		return '';
	}

	function getAddress() {
		return '';
	}

	function canPersist() {
		return true;
	}
	
	function getOrganization() {
		return SyndNodeLib::getInstance('null.null');
	}

	function getRoles() {
		return array();
	}
	
	static function resolveLogin($username) {
		$storage = SyndNodeLib::getDefaultStorage('user');
		$database = $storage->getDatabase();
		$sql = "
			SELECT u.node_id FROM synd_node_user u
			WHERE u.username = ".$database->quote($username);
		return $storage->getInstance($database->getOne($sql));
	}
	
	static function resolveEmail($email) {
		$storage = SyndNodeLib::getDefaultStorage('user');
		$database = $storage->getDatabase();
		$sql = "
			SELECT u.node_id FROM synd_node_user u
			WHERE u.info_email = ".$database->quote($email);
		return $storage->getInstance($database->getOne($sql));
	}

	function getIdentifier() {
		return $this->getEmail();
	}
	
	function getChildren() {
		return new EmptyIteratorAggregate();
	}

	static function findIdentifier($id) {
		return self::resolveEmail($id);
	}
	
	static function findInstances($query) {
		return SyndLib::runHook('search', 'user', $query);
	}
	
	function validatePassword($password) {
		return md5($password) == $this->data['PASSWORD'];
	}
	
	function getPersistentId($regenerate = false) {
		if (null == $this->data['INFO_PERSIST_ID'] || $regenerate) {
			$this->data['INFO_PERSIST_ID'] = md5(uniqid(''));
			$this->_save();
		}
		return $this->data['INFO_PERSIST_ID'];
	}

	function validate($data) {
		$errors = parent::validate($data);
		
		if (isset($data['USERNAME']) && !preg_match('/^[\w\-+\. @]{3,255}$/', $data['USERNAME']))
			$errors['USERNAME'] = SyndLib::translate("Invalid username given ('%s')", tpl_chop($data['USERNAME'],12));
		if (!empty($data['USERNAME'])) {
			$sql = "
				SELECT u.node_id FROM synd_node_user u
				WHERE LOWER(u.username) = ".$this->_db->quote(strtolower($data['USERNAME']));
			if (null != ($id = $this->_db->getOne($sql)) && $id != $this->nodeId)
				$errors['USERNAME'] = SyndLib::translate('The username already exists');
		}

		if (isset($data['INFO_EMAIL']) && !preg_match('/^[^@]+@.+\..+$/', $data['INFO_EMAIL']))
			$errors['INFO_EMAIL'] = SyndLib::translate("Invalid email address given ('%s')", tpl_chop($data['INFO_EMAIL'],12));
		
		if (isset($data['PASSWORD']) && (!isset($data['PASSWORD2']) || $data['PASSWORD2'] != $data['PASSWORD']))
			$errors['PASSWORD'] = SyndLib::translate('The passwords do not match');
		if (isset($data['PASSWORD']) && !preg_match('/^.{4,}$/', $data['PASSWORD']))
			$errors['PASSWORD'] = SyndLib::translate('Minimum password length is 4 characters');
			
		require_once 'core/lib/SyndHTML.class.inc';
		SyndHTML::validateFile($data, $errors, 'INFO_PHOTO');
		return $errors;
	}

	function merge($data) {
		if (isset($data['USERNAME']))
			$data['USERNAME'] = htmlspecialchars($data['USERNAME']);
		if (empty($this->data['INFO_HEAD']) && empty($data['INFO_HEAD']))
			$data['INFO_HEAD'] = $data['USERNAME'];

		if (isset($data['INFO_DESC'])) {
			$data['INFO_DESC'] = preg_replace('/(?<=\s) /', '', $data['INFO_DESC']);
			$data['INFO_DESC'] = preg_replace('/(?<=\s{2})\s/', '', $data['INFO_DESC']);
			$data['INFO_DESC'] = htmlspecialchars($data['INFO_DESC']);
		}

		if (!empty($data['INFO_PHOTO']['tmp_name']) &&
			null !== ($file = SyndType::factory('image', $data['INFO_PHOTO']['tmp_name'], $data['INFO_PHOTO']['name']))) {
			if (is_object($this->data['INFO_PHOTO']))
				$this->data['INFO_PHOTO']->delete();
			$file->setParent($this);
			$this->data['INFO_PHOTO'] = $file;
		}
		unset($data['INFO_PHOTO']);

		if (isset($data['PASSWORD']))
			$data['PASSWORD'] = md5($data['PASSWORD']);

		parent::merge($data);
	}
}
