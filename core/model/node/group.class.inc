<?php
require_once 'core/model/DatabaseEntity.class.inc';
require_once 'core/lib/auth/Role.class.inc';

class synd_node_group extends AbstractDatabaseEntity implements Role {
	/**
	 * Cached ids of subgroups
	 * @access	protected
	 */
	var $_children = null;

	/**
	 * Cached ids of members
	 * @access	protected
	 */
	var $_members = null;
	
	function __factory() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_group');
		$this->data['TS_CREATE'] = $this->data['TS_UPDATE'] = time();
		$this->data['CREATE_NODE_ID'] = $this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		parent::__factory();
	}

	function __clone() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_group');
		$this->data['TS_CREATE'] = time();
		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_group');
	}

	function hasPermission($user, $perm, $arg = null) {
		$creator = $this->getCreator();
		if ('read' == $perm && null != $arg)
			return false;
		if ('admin' == $perm && $creator->hasAuthorization($user, $this))
			return true;
		if ('append' == $perm && ($arg instanceof synd_node_relation))
			return $this->hasPermission($user, 'write');
		if ('append' == $perm && ($arg instanceof synd_node_user) && $this->isPermitted('write'))
			return true;
		if ('remove' == $perm && $this->isPermitted('write'))
			return true;
		if ('merge' == $perm && in_array($arg, array(
			'PARENT_NODE_ID','ACCEPT_CLASS_IDS','INFO_HEAD','INFO_DESC','INFO_BODY')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function toString() {
		if (null != $this->data['INFO_HEAD'])
			return $this->data['INFO_HEAD'];
		$parent = $this->getParent();
		return $parent->toString();
	}
	
	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function setParent($parent) {
		parent::setParent($parent);
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
		$this->_save();
	}

	function appendChild($child) {
		if (($child instanceof synd_node_group))
			return parent::appendChild($child);
		
		if (($child instanceof synd_node_relation))
			$child->setParent($this);
		else {
			$sql = "
				SELECT r.node_id FROM synd_relation r
				WHERE 
					r.parent_node_id = ".$this->_db->quote($this->nodeId)." AND 
					r.child_node_id = ".$this->_db->quote($child->nodeId);
			if (null == $this->_db->getOne($sql)) {
				$relation = SyndNodeLib::factory('relation');
				$relation->setParent($this);
				$relation->setChild($child);
				$relation->save();
			}
			if (null === $this->_members)
				$this->getMembers();
			$this->_members[] = $child->nodeId;
		}
		
		return $child;
	}
	
	function removeChild($child) {
		if (($child instanceof synd_node_group))
			return parent::removeChild($child);
		
		$sql = "
			SELECT r.node_id FROM synd_relation r
			WHERE 
				r.parent_node_id = ".$this->_db->quote($this->nodeId)." AND 
				r.child_node_id = ".$this->_db->quote($child->nodeId);
		SyndLib::invoke($this->_storage->getInstances($this->_db->getCol($sql)),'delete');
		
		if (null === $this->_members)
			$this->getMembers();
		while (false !== ($i = array_search($child->nodeId, $this->_members)))
			unset($this->_members[$i]);

		return $child;
	}
	
	function getChildren() {
		if (null === $this->_children) {
			$sql = "
				SELECT g.node_id FROM synd_group g
				WHERE g.parent_node_id = ".$this->_db->quote($this->nodeId)."
				ORDER BY g.info_head";
			$this->_children = (array)$this->_db->getCol($sql);
			$this->_node_onchange_internal();
		}
		return $this->_storage->getInstances($this->_children);
	}
	
	function getBranch() {
		$branch = array();
		$parent = $this->getParent();
		while (!$parent->isNull()) {
			$branch[$parent->nodeId] = $parent;
			$parent = $parent->getParent();
		}
		$branch = array_reverse($branch);
		$branch[$this->nodeId] = $this;
		return $branch;
	}
	
	function getGroups() {
		return $this->getChildren();
	}
	
	function getCreator() {
		if (null === ($creator = SyndNodeLib::getInstance($this->data['CREATE_NODE_ID'])))
			$creator = SyndNodeLib::getInstance('user_null.null');
		return $creator;
	}
	
	function getDescription() {
		return $this->data['INFO_DESC'];
	}

	function getRelations() {
		$sql = "
			SELECT r.node_id FROM synd_relation r
			WHERE r.parent_node_id = ".$this->_db->quote($this->nodeId);
		return $this->_storage->getInstances($this->_db->getCol($sql));
	}
	
	function isDescendant($group) {
		return $group->nodeId == $this->nodeId || !$group->isNull() && $this->isDescendant($group->getParent());
	}

	function hasAuthorization(Instance $user, Instance $subject) {
		return $this->isMember($user) || parent::hasAuthorization($user, $subject);
	}
	
	function isMember($user) {
		if (null === $this->_members)
			$this->getMembers();
		if (in_array($user->nodeId, $this->_members))
			return true;
		
		foreach (array_keys($groups = $this->getGroups()) as $key) {
			if ($groups[$key]->isMember($user))
				return true;
		}
		
		return false;
	}
	
	function getMembers() {
		if (null === $this->_members) {
			$sql = "
				SELECT r.child_node_id FROM synd_relation r
				WHERE r.parent_node_id = ".$this->_db->quote($this->nodeId);
			$this->_members = (array)$this->_db->getCol($sql);
			$this->_node_onchange_internal();
		}
		return SyndNodeLib::getInstances($this->_members);
	}
	
	function getMemberRelations() {
		return $this->getRelations();
	}
	
	function addMember($user) {
		return $child = $this->appendChild($user);
	}
	
	function removeMember($user) {
		return $child = $this->removeChild($user);
	}
	
	function removeMemberRecursive($user) {
		foreach (array_keys($groups = $this->getGroups()) as $key)
			$groups[$key]->removeMemberRecursive($user);
		return $child = $this->removeMember($user);
	}
	
	function getIndexDocument($index) {
		return $index->createComposite(array(
			$index->createFragment($this->toString(),'title'),
			$index->createFragment($this->data['INFO_DESC'],'description'),
			$index->createFragment($this->data['INFO_BODY'],'body'),
			));
	}

	function getIdentifier() {
		return $this->nodeId;
	}
	
	static function findIdentifier($id) {
		return self::getDefaultStorage()->getInstance($id);
	}
	
	static function findInstances($query) {
		return SyndLib::runHook('search', 'group', $query);
	}
	
	function validate($data) {
		$errors = parent::validate($data);
		if (isset($data['INFO_HEAD']) && null == trim($data['INFO_HEAD']))
			$errors['INFO_HEAD'] = SyndLib::translate('Please supply a title');
		return $errors;
	}
	
	function email($sender, $subject, $body, $recurse = true) {
		global $synd_user;
		if ($recurse) {
			foreach (array_keys($groups = $this->getGroups()) as $key) 
				$groups[$key]->email($sender, $subject, $body, $recurse);
		}
		
		$bcc = null;
		foreach (array_keys($members = $this->getMembers()) as $key) {
			$email = $members[$key]->getEmail();
			if (null != $email && $synd_user->getEmail() != $email)
				$bcc .= ",\r\n\t$email";
		}
		
		if (null != $bcc) {
			$bcc = trim($bcc, " \r\n\t,").",\r\n\t$sender";
			if (mail('', $subject, $body, "From: $sender\r\nBcc: $bcc"))
				return 'sent';
			return 'error';
		}
		
		return 'recipients';
	}
	
	function _view_email(Request $request, Template $page) {
		global $synd_user;
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (empty($request['subject']) || empty($request['body']))
			return;

		$sender = null != $synd_user->getEmail() ? 
			$synd_user->getEmail() : "noreply@{$_SERVER['SERVER_NAME']}";
		$result = $this->email($sender, 
			$request['subject'], $request['body'], !empty($request['recurse']));

		$return = tpl_uri_return();
		$return .= false===strpos($return,'?') ? '?' : '&';
		return array('redirect' => "{$return}result=$result");
	}

	function _node_onchange_child() {
		$this->_children = null;
		parent::_node_onchange_child();
	}
	
	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();
		$this->data['TS_UPDATE'] = time();
		$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
	}
}
