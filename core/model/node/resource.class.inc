<?php
require_once 'core/model/DatabaseEntity.class.inc';

/**
 * Project and budget planning resource
 *
 * Employees, consultants or other fixed resources are allocated to 
 * periods for project planning purposes.
 */
class synd_node_resource extends AbstractDatabaseEntity {
	function __factory() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_plan_resource');
		parent::__factory();
	}

	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_plan_resource');
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_plan_resource');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}
	
	function toString() {
		return $this->data['INFO_HEAD'];
	}

	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function setParent($parent) {
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
	}
}
