<?php
require_once 'core/model/DatabaseEntity.class.inc';

class synd_node_relation extends AbstractDatabaseEntity {
	function __construct($storage, $database, $data) {
		parent::__construct($storage, $database, $data);
		SyndNodeLib::preload($this->data['PARENT_NODE_ID']);
		SyndNodeLib::preload($this->data['CHILD_NODE_ID']);
	}
	
	function __factory() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_relation');
		$this->data['TS_CREATE'] = time();
		parent::__factory();
	}

	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_relation');
		$this->data['TS_CREATE'] = time();
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_relation');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}
	
	function toString() {
		$child = $this->getChild();
		return $child->toString(); 
	}

	function hasPermission($user, $perm, $arg = null) {
		$parent = $this->getParent();
		if ($parent->isPermitted($perm, $arg))
			return true;
		if ('write' == $perm && $parent->hasAuthorization($user, $this))
			return true;

		$child = $this->getChild();
		if ($child->isPermitted($perm, $arg))
			return true;
			
		return false;
	}

	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function setParent($node) {
		$this->data['PARENT_NODE_ID'] = $node->nodeId;
		$this->_save();
	}

	function getChild() {
		if (null === ($child = SyndNodeLib::getInstance($this->data['CHILD_NODE_ID'])))
			$child = SyndNodeLib::getInstance('null.null');
		return $child;
	}
	
	function setChild($node) {
		$this->data['CHILD_NODE_ID'] = $node->nodeId;
		$this->_save();
	}
	
	function setWeight($weight) {
		$this->data['INFO_WEIGHT'] = $weight;
		$this->_save();
	}

	function getWeight() {
		return $this->data['INFO_WEIGHT'];
	}

	function delete() {
		$sql = "
			DELETE FROM synd_relation
			WHERE 
				parent_node_id = ".$this->_db->quote($this->nodeId)." OR
				child_node_id = ".$this->_db->quote($this->nodeId);
		$this->_db->query($sql);
		parent::delete();
	}

	function _entity_onpreflush() {
		parent::_entity_onpreflush();
		$this->data['TS_UPDATE'] = time();
	}
}
