<?php
require_once 'core/model/DatabaseEntity.class.inc';
require_once 'core/lib/TemplateLib.inc';

class synd_node_node extends AbstractDatabaseEntity {
	var $_comments = null;
	
	function __construct($storage, $database, $data) {
		if (null != $data['DATA_PINGS'])
			$data['DATA_PINGS'] = unserialize($database->unwrapBinary($data['DATA_PINGS']));
		parent::__construct($storage, $database, $data);
	}
	
	function __factory() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_node');
		$this->data['CREATE_NODE_ID'] = $this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_CREATE'] = $this->data['TS_UPDATE'] = time();
		parent::__factory();
	}

	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_node');
		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_CREATE'] = time();
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_node');
	}
	
	function nodeId() {
		return $this->data['NODE_ID'];
	}
	
	function hasPermission($user, $perm, $arg = null) {
		if ('merge' == $perm && in_array($arg, array(
			'PARENT_NODE_ID','FLAG_MODERATE','FLAG_PROMOTE','FLAG_PINGBACKS','INFO_LANG')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}
	
	function getHandler() {
		return 'node';
	}

	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function setParent($parent) {
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
		$this->_save();
	}

	function getChildren() {
		$sql = "
			SELECT n.node_id FROM synd_node n
			WHERE n.parent_node_id = ".$this->_db->quote($this->nodeId);
		return $this->_storage->getInstances($this->_db->getCol($sql));
	}
	
	function getCustomer() {
		if (null != ($client = SyndNodeLib::getInstance($this->data['CREATE_NODE_ID'])))
			return $client;
		return SyndNodeLib::getInstance('user_null.null');
	}
	
	function getCreated() {
		return $this->data['TS_CREATE'];
	}

	function delete() {
		$sql = "
			DELETE FROM synd_relation
			WHERE 
				parent_node_id = ".$this->_db->quote($this->nodeId)." OR
				child_node_id = ".$this->_db->quote($this->nodeId);
		$this->_db->query($sql);

		SyndLib::invoke($this->getChildren(), 'delete');
		parent::delete();
	}

	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();
		$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_UPDATE'] = time();
	}
}
