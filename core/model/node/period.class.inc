<?php
require_once 'core/model/DatabaseEntity.class.inc';

/**
 * Project and budget planning timeperiod
 *
 * Projects have timeperiods for which they are active. The periods
 * are used in the planning process where resources are allocated
 * to projects and periods.
 */
class synd_node_period extends AbstractDatabaseEntity {
	function __factory() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_plan_period');
		parent::__factory();
	}

	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_plan_period');
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_plan_period');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function setParent($parent) {
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
	}
	
	function getBeginTime() {
		return $this->data['TS_START'];
	}
}
