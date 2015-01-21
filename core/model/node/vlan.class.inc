<?php
require_once 'core/model/DatabaseEntity.class.inc';
require_once 'core/model/node/interface.class.inc';

/**
 * Virtual LAN
 */
class synd_node_vlan extends AbstractDatabaseEntity {
	/**
	 * Cached network data
	 * @access	private
	 */
	var $_networks = null;
	
	function __factory() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_vlan');
		parent::__factory();
	}

	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_vlan');
		parent::__clone();
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_inv_vlan');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function getHandler() {
		return 'inventory';
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('merge' == $perm && in_array($arg, array(
			'PARENT_NODE_ID','INFO_HEAD','INFO_DESC')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}

	function toString() {
		return $this->data['INFO_HEAD'];
	}
	
	function getDescription() {
		return $this->data['INFO_DESC'];
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
	
	function getNetworks() {
		if (null === $this->_networks) {
			$sql = "
				SELECT n.* FROM synd_inv_vlan_network n
				WHERE n.parent_node_id = ".$this->_db->quote($this->nodeId);
			$this->_networks = (array)$this->_db->getAll($sql);
		}
		return $this->_networks;
	}
	
	/**
	 * Adds a network to this VLAN
	 * @param	string	Network IP-address
	 * @param	string	Netmask
	 */
	function addNetwork($network, $netmask) {
		if (null === $this->_networks)
			$this->getNetworks();
		$this->_networks[] = array(
			'NODE_ID' => 'network.'.$this->_db->nextId('synd_inv_network'),
			'PARENT_NODE_ID' => $this->nodeId,
			'INFO_NETWORK_ADDRESS' => $network,
			'INFO_NETWORK_MASK' => $netmask,
			'INFO_ENCODED_NET' => hexdec(dechex(
				synd_node_interface::ip2long($network) & 
				synd_node_interface::ip2long($netmask))),
			'INFO_ENCODED_MASK' => 
				synd_node_interface::ip2long($netmask),
			);
	}
	
	/**
	 * Returns the pager encapsulating the devices contained in this VLAN
	 * @return	PagedListing
	 */
	function getInterfacesListing() {
		require_once 'design/gui/PagedListing.class.inc';
		
		$query = $this->_db->createQuery();
		$n = $query->join('synd_inv_vlan_network', 'n');
		$query->where("$n.parent_node_id = ".$this->_db->quote($this->nodeId));

		$i = $query->join('synd_inv_interface', 'i');
		$query->where($this->_db->bitand("$i.info_ip_encoded", "$n.info_encoded_mask")." = $n.info_encoded_net");
		$query->column("$i.node_id");
		
		return new PagedListing($this->_storage, $query, array("$i.info_ip_encoded"), 256);
	}

	function getAvailableRanges() {
		$sql = "
			SELECT i.info_ip_encoded FROM synd_inv_vlan_network n, synd_inv_interface i
			WHERE 
				n.parent_node_id = ".$this->_db->quote($this->nodeId)." AND
				".$this->_db->bitand("i.info_ip_encoded", "n.info_encoded_mask")." = n.info_encoded_net";
		
		$allocated = $this->_db->getCol($sql);
		$ranges = array();
		
		foreach ($this->getNetworks() as $network) {
			$start = null;
			$first = $network['INFO_ENCODED_NET'] + 1;
			$last = $network['INFO_ENCODED_NET'] + bindec(strtr(decbin((float)$network['INFO_ENCODED_MASK']),'01','10')) - 1;
			
			for ($ip = $first; $ip <= $last; $ip++) {
				if (!in_array($ip, $allocated) && ($ip == $first || in_array($ip-1, $allocated)))
					$start = $ip;
				if (!in_array($ip, $allocated) && ($ip == $last || in_array($ip+1, $allocated)))
					$ranges[] = array($start, $ip);
			}
		}
		
		return $ranges;
	}
	
	function _view_delNetwork(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!isset($request['confirm'])) 
			return array('content' => $page->fetchnode($this, 'confirm_delete_network.tpl'));
		
		$sql = "
			DELETE FROM synd_inv_vlan_network
			WHERE 
				node_id = ".$this->_db->quote($request[2])." AND
				parent_node_id = ".$this->_db->quote($this->nodeId);
		$this->_db->query($sql);
		$this->_networks = null;
		$this->_node_onchange_internal();
	}

	function validate($data) {
		$errors = parent::validate($data);
		if (!empty($data['INFO_HEAD']) && !preg_match('/^[\w\-]+$/i', $data['INFO_HEAD']))
			$errors['INFO_HEAD'] = SyndLib::translate("Invalid VLAN name '%s', please use only character, digits and underscore", tpl_chop($data['INFO_HEAD'],16));
		if (isset($data['network']['INFO_NETWORK_ADDRESS'],$data['network']['INFO_NETWORK_MASK']) && count(array_filter($data['network'],'strlen'))) {
			if (!synd_node_interface::ipValidate($data['network']['INFO_NETWORK_ADDRESS']))
				$errors['INFO_NETWORK_ADDRESS'] = SyndLib::translate("Invalid IP-address '%s'", tpl_chop($data['network']['INFO_NETWORK_ADDRESS'],15));
			else if (!synd_node_interface::ipValidate($data['network']['INFO_NETWORK_MASK']))
				$errors['INFO_NETWORK_MASK'] = SyndLib::translate("Invalid netmask '%s'", tpl_chop($data['network']['INFO_NETWORK_MASK'],15));
			else {
				$net = hexdec(dechex(
					synd_node_interface::ip2long($data['network']['INFO_NETWORK_ADDRESS']) & 
					synd_node_interface::ip2long($data['network']['INFO_NETWORK_MASK'])));
				$mask = synd_node_interface::ip2long($data['network']['INFO_NETWORK_MASK']);
				
				$sql = "
					SELECT v.NODE_ID, v.INFO_HEAD, n.INFO_NETWORK_ADDRESS, n.INFO_NETWORK_MASK 
					FROM synd_inv_vlan v, synd_inv_vlan_network n
					WHERE 
						v.node_id = n.parent_node_id AND (
						".$this->_db->bitand('n.info_encoded_net', $mask)." = $net OR
						".$this->_db->bitand('n.info_encoded_mask', $net)." = n.info_encoded_net)";
				
				if (null !== ($conflict = $this->_db->getRow($sql))) {
					$errors['INFO_NETWORK_ADDRESS'] = SyndLib::translate("The network (%s/%s) collides with '%s' at (%s/%s)",
						$data['network']['INFO_NETWORK_ADDRESS'], $data['network']['INFO_NETWORK_MASK'],
						$conflict['INFO_HEAD'], $conflict['INFO_NETWORK_ADDRESS'], $conflict['INFO_NETWORK_MASK']);
				}
			}
		}
		return $errors;
	}
	
	function merge($data) {
		if (isset($data['network']['INFO_NETWORK_ADDRESS'],$data['network']['INFO_NETWORK_MASK']) && count(array_filter($data['network'],'strlen'))) {
			$this->addNetwork($data['network']['INFO_NETWORK_ADDRESS'], $data['network']['INFO_NETWORK_MASK']);
			$this->checkout();
		}
		parent::merge($data);
	}
	
	/**
	 * Finds an unallocated IP-address in this VLAN
	 * @param	string	Id of interface
	 * @return	string
	 */
	function _public_getAvailableIpAddress($interface) {
		// Check if interface exists on this VLAN already
		$sql = "
			SELECT i.info_ip_address FROM synd_inv_vlan_network n, synd_inv_interface i
			WHERE 
				n.parent_node_id = ".$this->_db->quote($this->nodeId)." AND
				i.node_id = ".$this->_db->quote($interface)." AND
				".$this->_db->bitand("i.info_ip_encoded", "n.info_encoded_mask")." = n.info_encoded_net";
		if (null != ($ip = $this->_db->getOne($sql)))
			return $ip;
			
		// Find available IP
		if (count($ranges = $this->getAvailableRanges()))
			return long2ip(reset(reset($ranges)));
		
		return '';
	}
	
	function _entity_onpostflush() {
		parent::_entity_onpostflush();
		foreach ((array)$this->_networks as $network)
			$this->_db->replace('synd_inv_vlan_network', $network);
	}
}
