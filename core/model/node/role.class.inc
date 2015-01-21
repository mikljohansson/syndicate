<?php
require_once 'core/model/DatabaseEntity.class.inc';
require_once 'core/lib/auth/Role.class.inc';

class synd_node_role extends AbstractDatabaseEntity implements Role {
	var $_members = null;

	function __factory() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_relation');
		parent::__factory();
	}

	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_relation');
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_access_role');
	}
	
	function nodeId() {
		return $this->data['NODE_ID'];
	}
	
	function toString() {
		return $this->data['INFO_HEAD'];
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('merge' == $perm && in_array($arg, array('INFO_HEAD','INFO_DESC')))
			return $this->hasPermission($user, 'write');
		$access = Module::getInstance('access');
		if ($access->hasPermission($user, $perm, $arg))
			return true;
		return parent::hasPermission($user, $perm, $arg);
	}

	function getDescription() {
		return $this->data['INFO_DESC'];
	}

	function appendChild($child) {
		if (!$this->isMember($child)) {
			$relation = SyndNodeLib::factory('relation');
			$relation->setParent($this);
			$relation->setChild($child);
			$relation->save();
			
			$this->_members[] = $child->nodeId;
			$this->_node_onchange_internal();
		}
		return $child;
	}
	
	function removeChild($child) {
		$sql = "
			DELETE FROM synd_relation
			WHERE
				parent_node_id = ".$this->_db->quote($this->nodeId)." AND
				child_node_id = ".$this->_db->quote($child->nodeId);
		$this->_db->query($sql);
		$this->_members = null;
		$this->_node_onchange_internal();
		return $child;
	}
	
	function getRelations() {
		$sql = "
			SELECT r.node_id FROM synd_relation r
			WHERE r.parent_node_id = ".$this->_db->quote($this->nodeId);
		return $this->_storage->getInstances($this->_db->getCol($sql));
	}
	
	function getMembers() {
		if (null !== $this->_members)
			return SyndNodeLib::getInstances($this->_members);
		
		$members = array();
		$relations = $this->getRelations();
		foreach (array_keys($relations) as $key) 
			$members[] = $relations[$key]->getChild();
			
		$this->_members = SyndLib::collect($members, 'nodeId');
		$this->_node_onchange_internal();
		
		return $members;
	}

	function isMember($user) {
		if (null !== $this->_members && in_array($user->nodeId, $this->_members) ||
			in_array($user->nodeId, SyndLib::collect($this->getMembers(),'nodeId')))
			return true;
		return false;
	}
	
	function hasAuthorization(Instance $user, Instance $subject) {
		if (null !== $this->_members && in_array($user->nodeId, $this->_members))
			return true;
		foreach (array_keys($members = $this->getMembers()) as $key) {
			if ($members[$key]->hasAuthorization($user, $subject))
				return true;
		}
		return parent::hasAuthorization($user, $subject);
	}

	function findRoles($query, $offset = 0, $limit = 20) {
		$access = Module::getInstance('access');
		return $access->findRoles($query, $offset, $limit);
	}

	function getIdentifier() {
		return $this->nodeId;
	}
	
	function getChildren() {
		return new EmptyIteratorAggregate();
	}

	static function findIdentifier($id) {
		return self::getDefaultStorage()->getInstance($id);
	}

	static function findInstances($query) {
		return SyndLib::runHook('search', 'role', $query);
	}

	function _view_add(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!empty($request['roles'])) {
			foreach ((array)$request['roles'] as $id) 
				$this->appendChild(SyndNodeLib::getInstance($id));
		}
	}

	function _view_remove(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		$this->removeChild(SyndNodeLib::getInstance($request[2]));
	}
}
