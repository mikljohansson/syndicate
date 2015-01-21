<?php
require_once 'core/model/DatabaseEntity.class.inc';

/**
 * Network interface card
 */
class synd_node_nic extends AbstractDatabaseEntity {
	function __construct($storage, $database, $data) {
		parent::__construct($storage, $database, $data);
		$this->_storage->preload($this->data['PARENT_NODE_ID']);
	}
	
	function __factory() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_nic');
		parent::__factory();
	}

	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_nic');
		parent::__clone();
	}
	
	function __wakeup() {
		parent::__wakeup();
		$this->_storage->preload($this->data['PARENT_NODE_ID']);
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_inv_nic');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function getHandler() {
		return 'inventory';
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('merge' == $perm && in_array($arg, array(
			'INFO_MAC_ADDRESS','INFO_HEAD')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}

	function toString() {
		return null != $this->getMacAddress() ? $this->getMacAddress() : parent::toString();
	}
	
	function setDescription($description) {
		$this->data['INFO_HEAD'] = $description;
	}
	
	function getDescription() {
		return $this->data['INFO_HEAD'];
	}

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
		return $index->createFragment($this->data['INFO_MAC_ADDRESS'].' '.$this->data['INFO_HEAD']);
	}
	
	/**
	 * Returns the configured MAC address
	 * @return	string
	 */
	function getMacAddress() {
		return $this->data['INFO_MAC_ADDRESS'];
	}
	
	function setMacAddress($mac) {
		$this->data['INFO_MAC_ADDRESS'] = strtoupper($mac);
		$this->_save();
	}
	
	/**
	 * Returns a macaddress in 01:02:03:04:05:06 notation
	 * @param string macaddress to fix
	 * @return	string
	 */
	static function fixMacAddress($mac) {
		return preg_replace('/\w{2}(?=\w)/S', '\0:', strtoupper($mac));
	}
	
	/**
	 * Returns the interface associated with this nic
	 * @return	synd_node_interface	Returns a synd_node_null if not found
	 */
	function getInterface() {
		$sql = "
			SELECT i.node_id FROM synd_inv_interface i
			WHERE i.nic_node_id = ".$this->_db->quote($this->nodeId);
		if (null === ($interface = $this->_storage->getInstance($this->_db->getOne($sql))))
			$interface = SyndNodeLib::getInstance('null.null');
		return $interface;
	}
	
	/**
	 * Returns the VLAN this NIC was last seen on
	 * @return	synd_node_vlan
	 */
	function getLastVirtualLan() {
		if (null === ($vlan = $this->_storage->getInstance($this->data['INFO_LAST_VLAN'])))
			$vlan = SyndNodeLib::getInstance('null.null');
		return $vlan;
	}
	
	/**
	 * Returns the last switch this NIC was seen on
	 * @return	string
	 */
	function getLastSwitch() {
		return $this->data['INFO_LAST_SWITCH'];
	}
	
	/**
	 * Returns the last switch port this NIC was seen on
	 * @return	string
	 */
	function getLastSwitchPort() {
		return $this->data['INFO_LAST_SWITCH_PORT'];
	}
	
	/**
	 * Returns the last timestamp this NIC was seen
	 * @return	integer	Returns -1 if NIC hasn't been seen
	 */
	function getLastSeen() {
		return null != $this->data['INFO_LAST_SEEN'] ? $this->data['INFO_LAST_SEEN'] : -1;
	}

	/**
	 * Returns the status of this NIC in {Unknown, Expired, Recent, Active}
	 * @return	string
	 */
	function getStatusName() {
		$ts = time();
		if (empty($this->data['INFO_LAST_SEEN']))
			return 'Unknown';
		if ($this->data['INFO_LAST_SEEN'] < $ts - 86400*60)
			return 'Expired';
		if ($this->data['INFO_LAST_SEEN'] < $ts - 86400*14)
			return 'Recent';
		return 'Active';
	}

	function validate($data) {
		$errors = parent::validate($data);
		if (isset($data['INFO_MAC_ADDRESS'])) {
			if (!preg_match('/^([a-f\d]{2}(:[a-f\d]{2}){5}|[a-f\d]{12})$/i', $data['INFO_MAC_ADDRESS']))
				$errors['INFO_MAC_ADDRESS'] = SyndLib::translate("Invalid MAC address '%s', please use '01:02:03:AB:CD:EF' notation", tpl_chop($data['INFO_MAC_ADDRESS'],16));
			else {
				$sql = "
					SELECT n.node_id FROM synd_inv_nic n
					WHERE 
						n.node_id != ".$this->_db->quote($this->nodeId)." AND
						UPPER(n.info_mac_address) = ".$this->_db->quote(strtoupper($data['INFO_MAC_ADDRESS']));
				if (null !== ($nic = $this->_storage->getInstance($this->_db->getOne($sql)))) {
					$computer = $nic->getParent();
					$errors['INFO_MAC_ADDRESS'] = SyndLib::translate("MAC address '%s' conflicts with computer '%s'", 
						tpl_chop($data['INFO_MAC_ADDRESS'],15), $computer->toString());
				}
			}
		}
		return $errors;
	}
	
	function merge($data) {
		if (isset($data['INFO_MAC_ADDRESS']))
			$data['INFO_MAC_ADDRESS'] = synd_node_nic::fixMacAddress($data['INFO_MAC_ADDRESS']);
		parent::merge($data);
	}
}
