<?php
require_once 'core/model/node/item.class.inc';

class synd_node_computer extends synd_node_item {
	/**
	 * Cached id of configuration
	 * @access	protected
	 */
	var $_config = null;

	/**
	 * Cached ids of operating systems
	 * @access	protected
	 */
	var $_oses = null;

	/**
	 * Cached ids of network interfaces
	 * @access	protected
	 */
	var $_nics = null;

	function __construct($storage, $database, $data) {
		if (null != $data['DATA_DISK_DRIVES'])
			$data['DATA_DISK_DRIVES'] = unserialize($database->unwrapBinary($data['DATA_DISK_DRIVES']));
		if (null != $data['DATA_ROM_DRIVES'])
			$data['DATA_ROM_DRIVES'] = unserialize($database->unwrapBinary($data['DATA_ROM_DRIVES']));
		if (null != $data['DATA_NETWORK_CARDS'])
			$data['DATA_NETWORK_CARDS'] = unserialize($database->unwrapBinary($data['DATA_NETWORK_CARDS']));
		if (null != $data['DATA_GRAPHIC_CARDS'])
			$data['DATA_GRAPHIC_CARDS'] = unserialize($database->unwrapBinary($data['DATA_GRAPHIC_CARDS']));
		if (null != $data['DATA_SOUND_CARDS'])
			$data['DATA_SOUND_CARDS'] = unserialize($database->unwrapBinary($data['DATA_SOUND_CARDS']));
		if (null != $data['DATA_MONITORS'])
			$data['DATA_MONITORS'] = unserialize($database->unwrapBinary($data['DATA_MONITORS']));

		parent::__construct($storage, $database, $data);
		$this->_storage->preload($this->data['PARENT_NODE_ID']);
	}
	
	function __wakeup() {
		parent::__wakeup();
		$this->_storage->preload($this->data['PARENT_NODE_ID']);
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		$query = parent::getEntityQuery($storage);
		$query->join('synd_inv_computer');
		return $query;
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('read' == $perm && null != $arg)
			return false;
		if ('append' == $perm && ($arg instanceof synd_node_os))
			return true;
		if ('invoke' == $perm && in_array($arg, array(
			'getoperatingsystem','setcurrentuser','setmotherboard','setphysicalmemory','setprocessor',
			'setkeyboard','adddiskdrive','addromdrive','addmonitor','addsounddevice','addnetworkdevice',
			'addvideocontroller','setremotetransport')))
			return true;
		return parent::hasPermission($user, $perm, $arg);
	}
	
	function appendChild($child) {
		if (($child instanceof synd_node_os)) {
			if (null === $this->_oses)
				$this->getOperatingSystems();
			$this->_oses[] = $child->nodeId;
			$this->_node_onchange_internal();
		}
		else if (($child instanceof synd_node_nic)) {
			if (null === $this->_nics)
				$this->getNetworkInterfaces();
			$this->_nics[] = $child->nodeId;
			$this->_node_onchange_internal();
		}
		return parent::appendChild($child);
	}
	
	function save() {
		parent::save();
		SyndLib::invoke($this->getNetworkInterfaces(), 'save');
		$config = $this->getConfig();
		$config->save();
	}

	function checkout() {
		parent::checkout();
		$config = $this->getConfig();
		$config->checkout();
		SyndLib::invoke($this->getNetworkInterfaces(), 'checkout');
	}
	
	function getTitle() {
		if (null != ($title = parent::getTitle()))
			return $title;
		
		// Default title to machinename from agent
		if (null === $this->_oses)
			$this->getOperatingSystems();
		if (null !== ($os = $this->_storage->getInstance(reset($this->_oses))) && null != $os->data['INFO_MACHINE_NAME'])
			return $os->data['INFO_MACHINE_NAME'];
		
		// Default to configured hostname
		$config = $this->getConfig();
		if (count($interfaces = $config->getInterfaces()) && null != ($host = $interfaces[key($interfaces)]->getHostname()))
			return $host;
		
		return SyndLib::translate('Unknown model');
	}
	
	/**
	 * Returns the active configuration
	 * @return	synd_node_config
	 */
	function getConfig() {
		if (null === $this->_config) {
			$sql = "
				SELECT c.node_id FROM synd_inv_config c
				WHERE c.parent_node_id = ".$this->_db->quote($this->nodeId);
			$this->_config = (string)$this->_db->getOne($sql);
			$this->_node_onchange_internal();
		}

		if ('' == $this->_config || null === ($config = $this->_storage->getInstance($this->_config))) {
			$config = $this->appendChild($this->_storage->factory('config'));
			$this->_config = $config->nodeId;
			$this->_node_onchange_internal();
		}

		return $config;
	}
	
	/**
	 * Returns network interfaces
	 * @return	array
	 */
	function getNetworkInterfaces() {
		if (null === $this->_nics) {
			$sql = "
				SELECT n.node_id FROM synd_inv_nic n
				WHERE n.parent_node_id = ".$this->_db->quote($this->nodeId);
			$this->_nics = (array)$this->_db->getCol($sql);
			$this->_node_onchange_internal();
		}
		return $this->_storage->getInstances($this->_nics);
	}

	/**
	 * Finds a network interface
	 * @access	private
	 * @param	string	Interface data to search for
	 * @return	synd_node_interface
	 */
	function _findNetworkInterface($data) {
		foreach (array_keys($nics = $this->getNetworkInterfaces()) as $key) {
			if ($nics[$key]->getMacAddress() == strtoupper($data['INFO_MAC_ADDRESS']))
				return $nics[$key];
		}
		$result = null;
		return $result;
	}
	
	/**
	 * Finds an interface
	 * @access	private
	 * @param	string	Interface data to search for
	 * @return	synd_node_interface
	 */
	function _findInterface($data) {
		$config = $this->getConfig();
		if (!$config->isNull()) {
			foreach (array_keys($interfaces = $config->getInterfaces()) as $key) {
				if ($interfaces[$key]->getIpAddress() == $data['INFO_IP_ADDRESS'])
					return $interfaces[$key];
			}
		}
		$result = null;
		return $result;
	}

	/**
	 * Finds a network interface
	 * @param	string			Hardware address to find.
	 * @return	synd_node_nic	Return null when nothing found
	 */
	function findNetworkInterface($mac) {
		foreach (array_keys($interfaces = $this->getNetworkInterfaces()) as $key) {
			if ($interfaces[$key]->getMacAddress() == strtoupper($mac))
				return $interfaces[$key];
		}
		$result = null;
		return $result;
	}

	function getCompositeData() {
		$data = parent::getCompositeData();
		foreach (array_keys($nics = $this->getNetworkInterfaces()) as $key)
			$data['nics'][$nics[$key]->nodeId] = $nics[$key]->getCompositeData();
		$config = $this->getConfig();
		foreach (array_keys($interfaces = $config->getInterfaces()) as $key)
			$data['interfaces'][$interfaces[$key]->nodeId] = $interfaces[$key]->getCompositeData();
		return $data;
	}
	
	function validate($data) {
		$errors = parent::validate($data);

		if (!empty($data['nics'])) {
			foreach ((array)$data['nics'] as $id => $ifdata) {
				if (null !== ($nic = $this->_storage->getInstance($id))) {
					$parent = $nic->getParent();
					if ($parent->nodeId != $this->nodeId)
						$errors['nic'] = SyndLib::translate('Invalid NIC specified');
				}
				else if (count(array_filter($ifdata,'strlen'))) {
					$nic = $this->appendChild($this->_storage->factory('nic'));
					$nic->merge($ifdata);
				}
				
				if (isset($nic)) {
					$errors = array_merge($errors, $nic->validate($ifdata));
					unset($nic);
				}
			}
		}

		if (!empty($data['interfaces'])) {
			foreach ((array)$data['interfaces'] as $id => $ifdata) {
				$config = $this->getConfig();
				
				if (null !== ($interface = $this->_storage->getInstance($id))) {
					$parent = $interface->getParent();
					if ($parent->nodeId != $config->nodeId)
						$errors['interface'] = SyndLib::translate('Invalid interface specified');
				}
				else if (!empty($ifdata['INFO_HOSTNAME']) || !empty($ifdata['INFO_IP_ADDRESS'])) {
					$interface = $config->appendChild($config->_storage->factory('interface'));
					$interface->merge($ifdata);
				}
				
				if (isset($interface)) {
					$errors = array_merge($errors, $interface->validate($ifdata));
					unset($interface);
				}
			}
		}
		
		return $errors;
	}

	function merge($data) {
		if (!empty($data['nics'])) {
			foreach ((array)$data['nics'] as $id => $ifdata) {
				if (null !== ($nic = $this->_storage->getInstance($id)) || null !== ($nic = $this->_findNetworkInterface($ifdata)))
					$nic->merge($ifdata);
			}
		}
		
		if (!empty($data['interfaces'])) {
			foreach ((array)$data['interfaces'] as $id => $ifdata) {
				if (null !== ($interface = $this->_storage->getInstance($id)) || null !== ($interface = $this->_findInterface($ifdata)))
					$interface->merge($ifdata);
			}
		}

		parent::merge($data);
	}

	function _view_runAgent(Request $request, Template $page) {
		require_once 'core/lib/rpc/RpcTransport.class.inc';
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();

		$result = 'none';
		if ($this->isListening()) {
			$transport = Activator::getTransport(
				$this->data['INFO_REMOTE_METHOD'].':'.$this->data['INFO_REMOTE_URI']);
			if (!empty($this->data['INFO_REMOTE_ACTION']))
				$transport->setAction($this->data['INFO_REMOTE_ACTION']);
			if (!is_array($r = $transport->invoke('run')))
				$result = $r && 'false' != $r ? 'ok' : 'Already running';
			else
				$result = $r['message'];
		}
		
		return array('redirect' => tpl_uri_return().'?agent='.rawurlencode($result));
	}
	
	/**
	 * Is the agent listening?
	 * @return	bool
	 */
	function isListening() {
		return !empty($this->data['INFO_REMOTE_METHOD']) && !empty($this->data['INFO_REMOTE_URI']);
	}
	
	function _public_begin() {}
	
	function _public_commit() {
		if ($this->isNew())
			$this->save();
		$this->_filter($this->data['DATA_DISK_DRIVES']);
		$this->_filter($this->data['DATA_ROM_DRIVES']);
		$this->_filter($this->data['DATA_NETWORK_CARDS']);
		$this->_filter($this->data['DATA_GRAPHIC_CARDS']);
		$this->_filter($this->data['DATA_SOUND_CARDS']);
		$this->_filter($this->data['DATA_MONITORS']);
	}
	
	/**
	 * Removes old entries based on TS_UPDATE timestamp
	 * @access	private
	 */
	function _filter(&$values) {
		if (is_array($values)) {
			foreach (array_keys($values) as $key) {
				if (!isset($values[$key]['TS_UPDATE']) || $values[$key]['TS_UPDATE'] < time()-1800) {
					unset($values[$key]);
					$this->save();
				}
			}
		}
	}
	
	/**
	 * @access	private
	 */
	function _implode($a) {
		if (is_array($a)) {
			foreach ($a as $key => $value)
				$a[$key] = $this->_implode($value);
			return implode(' ', $a);
		}
		return $a;
	}

	function getIndexDocument($index) {
		$fragments = array(
			parent::getIndexDocument($index),
			$index->createFragment(
				$this->data['INFO_MB_MAKE'].' '.
				$this->data['INFO_MB_BIOS'].' '.
				$this->data['INFO_MB_SERIAL'].' '.
				$this->data['INFO_KEYBOARD'].' '.
				$this->data['INFO_CPU_DESC'].' '.
				$this->data['INFO_CPU_ID'].' '.
				$this->_implode($this->getDiskDrives()).' '.
				$this->_implode($this->getROMDrives()).' '.
				$this->_implode($this->getSoundDevices()).' '.
				$this->_implode($this->getNetworkDevices()).' '.
				$this->_implode($this->getVideoControllers()).' '.
				$this->_implode($this->getMonitors())),
			);
			
		foreach ($this->getOperatingSystems() as $os) 
			$fragments[] = $os->getIndexDocument($index);
		foreach ($this->getNetworkInterfaces() as $nic) 
			$fragments[] = $nic->getIndexDocument($index);
		$fragments[] = $this->getConfig()->getIndexDocument($index);

		return $index->createComposite($fragments);
	}

	function getOperatingSystems() {
		if (null === $this->_oses) {
			$sql = "
				SELECT o.node_id FROM synd_inv_os o
				WHERE o.parent_node_id = ".$this->_db->quote($this->nodeId);
			$this->_oses = (array)$this->_db->getCol($sql);
			$this->_node_onchange_internal();
		}
		return $this->_storage->getInstances($this->_oses);
	}
	
	function setRemoteTransport($method, $uri, $action = null, $version = '20050908') {
		if ($method != $this->data['INFO_REMOTE_METHOD']) {
			$this->data['INFO_REMOTE_METHOD'] = $method;
			$this->_save();
		}
		if ($uri != $this->data['INFO_REMOTE_URI']) {
			$this->data['INFO_REMOTE_URI'] = $uri;
			$this->_save();
		}
		if ($action != $this->data['INFO_REMOTE_ACTION']) {
			$this->data['INFO_REMOTE_ACTION'] = $action;
			$this->_save();
		}
		if ($version != $this->data['INFO_REMOTE_VERSION']) {
			$this->data['INFO_REMOTE_VERSION'] = $version;
			$this->_save();
		}
	}
	
	function setMotherboard($manufacturer, $bios, $serial = null) {
		if ($manufacturer != $this->data['INFO_MB_MAKE']) {
			$this->data['INFO_MB_MAKE'] = $manufacturer;
			$this->_save();
		}
		if ($bios != $this->data['INFO_MB_BIOS']) {
			$this->data['INFO_MB_BIOS'] = $bios;
			$this->_save();
		}
		if (trim($serial) != $this->data['INFO_MB_SERIAL']) {
			$this->data['INFO_MB_SERIAL'] = trim($serial);
			$this->_save();
		}
	}

	function setPhysicalMemory($size) {
		if ($size != $this->data['INFO_RAM']) {
			$this->data['INFO_RAM'] = $size;
			$this->_save();
		}
	}

	function setProcessor($description, $clock, $count = 1, $cpuid = null) {
		if ($description != $this->data['INFO_CPU_DESC']) {
			$this->data['INFO_CPU_DESC'] = $description;
			$this->_save();
		}
		if ($clock != $this->data['INFO_CPU_CLOCK']) {
			$this->data['INFO_CPU_CLOCK'] = $clock;
			$this->_save();
		}
		if ($count != $this->data['INFO_CPU_COUNT']) {
			$this->data['INFO_CPU_COUNT'] = $count;
			$this->_save();
		}
		if ($cpuid != $this->data['INFO_CPU_ID']) {
			$this->data['INFO_CPU_ID'] = $cpuid;
			$this->_save();
		}
	}
	
	function setKeyboard($description) {
		if ($description != $this->data['INFO_KEYBOARD']) {
			$this->data['INFO_KEYBOARD'] = $description;
			$this->_save();
		}
	}

	function addDiskDrive($description, $size, $cache = null, $device = null) {
		$info = array(
			'INFO_DEVICE' => $device,
			'INFO_DESC' => $description,
			'INFO_SIZE' => (int)$size,
			'INFO_CACHE' => (int)$cache);
			
		if (!isset($this->data['DATA_DISK_DRIVES'][$hash = SyndLib::hash($info)])) {
			$this->data['DATA_DISK_DRIVES'][$hash] = $info;
			$this->data['DATA_DISK_DRIVES'][$hash]['TS_UPDATE'] = time();
			$this->_save();
		}
	}

	function addROMDrive($description, $mountpoint) {
		$info = array(
			'INFO_DESC' => $description,
			'INFO_MOUNTPOINT' => $mountpoint);

		if (!isset($this->data['DATA_ROM_DRIVES'][$hash = SyndLib::hash($info)])) {
			$this->data['DATA_ROM_DRIVES'][$hash] = $info;
			$this->data['DATA_ROM_DRIVES'][$hash]['TS_UPDATE'] = time();
			$this->_save();
		}
	}

	function addSoundDevice($description) {
		$info = array('INFO_DESC' => $description);

		if (!isset($this->data['DATA_SOUND_CARDS'][$hash = SyndLib::hash($info)])) {
			$this->data['DATA_SOUND_CARDS'][$hash] = $info;
			$this->data['DATA_SOUND_CARDS'][$hash]['TS_UPDATE'] = time();
			$this->_save();
		}
	}

	function addNetworkDevice($description, $ipaddress, $macaddress) {
		$info = array(
			'INFO_DESC' => $description,
			'INFO_IP' 	=> $ipaddress,
			'INFO_MAC' 	=> strtoupper($macaddress));

		if (!isset($this->data['DATA_NETWORK_CARDS'][strtoupper($macaddress)])) {
			$this->data['DATA_NETWORK_CARDS'][strtoupper($macaddress)] = $info;
			$this->data['DATA_NETWORK_CARDS'][strtoupper($macaddress)]['TS_UPDATE'] = time();
			$this->_save();
		}
	}

	function addVideoController($description, $memory, $hres, $vres, $bits, $freq) {
		$info = array(
			'INFO_DESC' => $description,
			'INFO_RAM'  => (int)$memory,
			'INFO_HRES' => (int)$hres,
			'INFO_VRES' => (int)$vres,
			'INFO_BITS' => (int)$bits,
			'INFO_FREQ' => (int)$freq);

		if (!isset($this->data['DATA_GRAPHIC_CARDS'][$hash = SyndLib::hash($info)])) {
			$this->data['DATA_GRAPHIC_CARDS'][$hash] = $info;
			$this->data['DATA_GRAPHIC_CARDS'][$hash]['TS_UPDATE'] = time();
			$this->_save();
		}
	}

	function addMonitor($description, $vendorId, $size) {
		if (preg_match('/^DISPLAY\\\\([^\\\\]+)\\\\/', $vendorId, $matches))
			$vendorId = $matches[1];
		$info = array(
			'INFO_DESC' 	 => $description,
			'INFO_VENDOR_ID' => $vendorId,
			'INFO_SIZE' 	 => (int)$size);

		if (!isset($this->data['DATA_MONITORS'][$hash = SyndLib::hash($info)])) {
			$this->data['DATA_MONITORS'][$hash] = $info;
			$this->data['DATA_MONITORS'][$hash]['TS_UPDATE'] = time();
			$this->_save();
		}
	}

	function getDiskDrives() {
		if (null != $this->data['DATA_DISK_DRIVES'])
			return $this->data['DATA_DISK_DRIVES'];
		return array();
	}

	function getROMDrives() {
		if (null != $this->data['DATA_ROM_DRIVES'])
			return $this->data['DATA_ROM_DRIVES'];
		return array();
	}

	function getSoundDevices() {
		if (null != $this->data['DATA_SOUND_CARDS'])
			return $this->data['DATA_SOUND_CARDS'];
		return array();
	}
	
	function getNetworkDevices() {
		if (null != $this->data['DATA_NETWORK_CARDS'])
			return $this->data['DATA_NETWORK_CARDS'];
		return array();
	}

	function getVideoControllers() {
		if (null != $this->data['DATA_GRAPHIC_CARDS'])
			return $this->data['DATA_GRAPHIC_CARDS'];
		return array();
	}

	function getMonitors() {
		if (null != $this->data['DATA_MONITORS'])
			return $this->data['DATA_MONITORS'];
		return array();
	}
}
