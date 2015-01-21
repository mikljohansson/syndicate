<?php
require_once 'core/model/DatabaseEntity.class.inc';

/**
 * Service level agreement
 *
 * Leases have a SLA that specifies the level of service the client
 * has for the leased items
 */
class synd_node_sld extends AbstractDatabaseEntity {
	function __factory() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_sld');
		$this->data['CREATE_NODE_ID'] = $this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_CREATE'] = $this->data['TS_UPDATE'] = time();
		parent::__factory();
	}

	function __clone() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_sld');
		$this->data['CREATE_NODE_ID'] = $this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_CREATE'] = $this->data['TS_UPDATE'] = time();
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_inv_sld');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}
	
	function hasPermission($user, $perm, $arg = null) {
		if ('merge' == $perm && in_array($arg, array(
			'PARENT_NODE_ID','INFO_HEAD','INFO_DESC','INFO_URI')))
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
	
	function setTitle($title) {
		$this->data['INFO_HEAD'] = $title;
	}
	
	function getDescription() {
		return $this->data['INFO_DESC'];
	}
	
	/**
	 * Returns the query to select the current agreements that have this SLD
	 * @return	SyndDBQuery
	 */
	function getAgreementsQuery() {
		$query = $this->_db->createQuery();
		$l = $query->join('synd_inv_lease', 'l');
		$ls = $query->join('synd_inv_lease_sld', 'ls');

		$query->column("$l.node_id");
		$query->where("$l.node_id = $ls.lease_node_id");
		$query->where("$ls.sld_node_id = ".$this->_db->quote($this->nodeId));
		$query->where("$l.ts_terminated IS NULL AND $ls.ts_terminated IS NULL");

		return $query;
	}
	
	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();
		$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_UPDATE'] = time();
	}
}
