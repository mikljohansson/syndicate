<?php
require_once 'core/model/DatabaseEntity.class.inc';

/**
 * Computer interface configuration
 */
class synd_node_interface extends AbstractDatabaseEntity {
	function __construct($storage, $database, $data) {
		parent::__construct($storage, $database, $data);
		$this->_storage->preload($this->data['NIC_NODE_ID']);
	}
	
	function __factory() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_interface');
		parent::__factory();
	}

	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_interface');
		parent::__clone();
	}
	
	function __wakeup() {
		parent::__wakeup();
		$this->_storage->preload($this->data['NIC_NODE_ID']);
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_inv_interface');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function getHandler() {
		return 'inventory';
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('merge' == $perm && in_array($arg, array(
			'NIC_NODE_ID','INFO_HOSTNAME','INFO_IP_ADDRESS')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}

	/**
	 * @return	synd_node_config
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
		return $index->createFragment($this->data['INFO_IP_ADDRESS'].' '.$this->data['INFO_HOSTNAME']);
	}
	
	function getNetworkInterface() {
		if (null === ($nic = $this->_storage->getInstance($this->data['NIC_NODE_ID'])))
			$nic = SyndNodeLib::getInstance('null.null');
		return $nic;
	}
	
	function setNetworkInterface($nic) {
		$this->data['NIC_NODE_ID'] = $nic->nodeId;
		$this->_save();
	}
	
	/**
	 * Finds an interface by ip
	 * @param	string				IP-address to resolve
	 * @return	synd_node_interface	Returns synd_node_null on nothing found
	 */
	static function findInterface($ip) {
		$storage = SyndNodeLib::getDefaultStorage('interface');
		$database = $storage->getDatabase();
		$sql ="
			SELECT i.node_id FROM synd_inv_interface i
			WHERE i.info_ip_address = ".$database->quote($ip);
		if (null === ($interface = $storage->getInstance($database->getOne($sql))))
			$interface = SyndNodeLib::getInstance('null.null');
		return $interface;
	}
			
	function getHostname() {
		return $this->data['INFO_HOSTNAME'];
	}
	
	function setHostname($hostname) {
		$this->data['INFO_HOSTNAME'] = $hostname;
		$this->_save();
	}
	
	function getIpAddress() {
		return $this->data['INFO_IP_ADDRESS'];
	}
	
	function setIpAddress($ip) {
		$this->data['INFO_IP_ADDRESS'] = $ip;
		$this->data['INFO_IP_ENCODED'] = null != $ip ? synd_node_interface::ip2long($ip) : null;
		$this->_save();
	}
	
	function getVirtualLan() {
		if (null == $this->data['INFO_IP_ENCODED'])
			$vlan = SyndNodeLib::getInstance('null.null');
		else {
			$sql = "
				SELECT n.parent_node_id FROM synd_inv_vlan_network n
				WHERE ".$this->_db->bitand($this->data['INFO_IP_ENCODED'], "n.info_encoded_mask")." = n.info_encoded_net";
			if (null === ($vlan = $this->_storage->getInstance($this->_db->getOne($sql))))
				$vlan = SyndNodeLib::getInstance('null.null');
		}
		return $vlan;
	}
	
	function validate($data) {
		$errors = parent::validate($data);
		if (!empty($data['INFO_IP_ADDRESS'])) {
			if (!synd_node_interface::ipValidate($data['INFO_IP_ADDRESS']))
				$errors['INFO_IP_ADDRESS'] = SyndLib::translate("Invalid IP address '%s'", tpl_chop($data['INFO_IP_ADDRESS'],15));
			else {
				$sql = "
					SELECT i.node_id FROM synd_inv_interface i
					WHERE 
						i.node_id != ".$this->_db->quote($this->nodeId)." AND
						i.info_ip_address = ".$this->_db->quote($data['INFO_IP_ADDRESS']);
				if (null !== ($interface = $this->_storage->getInstance($this->_db->getOne($sql)))) {
					$config = $interface->getParent();
					$computer = $config->getParent();
					$errors['INFO_IP_ADDRESS'] = SyndLib::translate("IP address '%s' conflicts with computer '%s'", 
						tpl_chop($data['INFO_IP_ADDRESS'],15), $computer->toString());
				}
			}
		}
		
		if (!empty($data['INFO_HOSTNAME'])) {
			if (!preg_match('/^[\w\-\.]*$/', $data['INFO_HOSTNAME']))
				$errors['INFO_HOSTNAME'] = SyndLib::translate("Invalid hostname '%s'", tpl_chop($data['INFO_HOSTNAME'],15));
			else {
				$sql = "
					SELECT i.node_id FROM synd_inv_interface i
					WHERE 
						i.node_id != ".$this->_db->quote($this->nodeId)." AND
						LOWER(i.info_hostname) = ".$this->_db->quote(strtolower($data['INFO_HOSTNAME']));
				if (null !== ($interface = $this->_storage->getInstance($this->_db->getOne($sql)))) {
					$config = $interface->getParent();
					$computer = $config->getParent();
					$errors['INFO_HOSTNAME'] = SyndLib::translate("Hostname '%s' conflicts with computer '%s'", 
						tpl_chop($data['INFO_HOSTNAME'],15), $computer->toString());
				}
			}
		}
		
		return $errors;
	}
	
	function merge($data) {
		if (isset($data['INFO_IP_ADDRESS'])) {
			if ('' == $data['INFO_IP_ADDRESS'] && !empty($data['INFO_HOSTNAME']) && 
				$data['INFO_HOSTNAME'] != ($ip = gethostbyname($data['INFO_HOSTNAME'])))
				$data['INFO_IP_ADDRESS'] = $ip;
			$data['INFO_IP_ENCODED'] = null != $data['INFO_IP_ADDRESS'] ? 
				synd_node_interface::ip2long($data['INFO_IP_ADDRESS']) : null;
		}
		
		if (isset($data['INFO_HOSTNAME'])) {
			if ('' == $data['INFO_HOSTNAME'] && !empty($data['INFO_IP_ADDRESS']) && 
				$data['INFO_IP_ADDRESS'] != ($host = gethostbyaddr($data['INFO_IP_ADDRESS'])))
				$data['INFO_HOSTNAME'] = $host;
			$data['INFO_HOSTNAME'] = strtolower($data['INFO_HOSTNAME']);
		}
		
		parent::merge($data);
	}
	
	/**
	 * The ip2long function returns signed integers which database bitwise AND don't like
	 * @see		ip2long
	 * @return	unsigned int
	 */
	static function ip2long($ip) {
		$result = null;
		for ($i=3, $c=explode('.',$ip); $i>=0; $i--)
			$result += array_shift($c) * pow(2,8*$i);
		return $result;
	}

	/**
	 */
	static function ipValidate($ip) {
		if (!preg_match('/^\d{1,3}(\.\d{1,3}){3}$/i', $ip))
			return false;
		foreach (explode('.', $ip) as $net) {
			if ($net > 255)
				return false;
		}
		return true;
	}
}
