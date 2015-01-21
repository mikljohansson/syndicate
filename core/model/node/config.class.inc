<?php
require_once 'core/model/DatabaseEntity.class.inc';

/**
 * Computer config
 */
class synd_node_config extends AbstractDatabaseEntity {
	/**
	 * Buffered ids of interfaces
	 * @access	protected
	 */
	var $_interfaces = null;
	
	function __factory() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_config');
		$this->data['TS_CREATE'] = $this->data['TS_UPDATE'] = time();
		$this->data['CREATE_NODE_ID'] = $this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		parent::__factory();
	}

	function __clone() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_config');
		$this->data['TS_CREATE'] = time();
		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;
		parent::__clone();
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_inv_config');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function getHandler() {
		return 'inventory';
	}

	/**
	 * @return	synd_node_computer
	 */
	function getParent() {
		if (null === ($parent = $this->_storage->getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function setParent($parent) {
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
		$this->_save();
	}

	function getIndexDocument($index) {
		if (!count($interfaces = $this->getInterfaces()))
			return null;
		$fragments = array();
		foreach (array_keys($interfaces) as $key)
			$fragments[] = $interfaces[$key]->getIndexDocument($index);
		return $index->createComposite($fragments);
	}
	
	function appendChild($child) {
		if (($child instanceof synd_node_interface)) {
			$this->_interfaces[] = $child->nodeId;
			$this->_node_onchange_internal();
		}
		return parent::appendChild($child);
	}
	
	function save() {
		parent::save();
		SyndLib::invoke($this->getInterfaces(), 'save');
	}

	function checkout() {
		parent::checkout();
		SyndLib::invoke($this->getInterfaces(), 'checkout');
	}
	
	/**
	 * Returns interfaces associated with this configuration
	 * @see		synd_node_interface
	 * @return	array
	 */
	function getInterfaces() {
		if (null === $this->_interfaces) {
			$sql = "
				SELECT i.node_id FROM synd_inv_interface i
				WHERE i.parent_node_id = ".$this->_db->quote($this->nodeId);
			$this->_interfaces = (array)$this->_db->getCol($sql);
			$this->_node_onchange_internal();
		}
		return $this->_storage->getInstances($this->_interfaces);
	}
	
	/**
	 * Attempts to locate an interface
	 * @param	string	IP address to find
	 * @return	synd_node_interface	Return null when nothing found
	 */
	function findInterface($ip) {
		foreach (array_keys($interfaces = $this->getInterfaces()) as $key) {
			if ($interfaces[$key]->getIpAddress() == $ip)
				return $interfaces[$key];
		}
		$result = null;
		return $result;
	}

	/**
	 * Returns available VLANs
	 * @see		synd_node_vlan
	 * @return	array
	 */
	function getVirtualLans() {
		$sql = "SELECT v.node_id FROM synd_inv_vlan v";
		return $this->_storage->getInstances($this->_db->getCol($sql));
	}

	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();
		$this->data['TS_UPDATE'] = time();
		$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
	}
}
