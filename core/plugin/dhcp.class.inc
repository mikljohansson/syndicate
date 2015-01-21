<?php
require_once 'core/Plugin.class.inc';

/**
 * DHCP inventory plugin 
 *
 * Integates DHCP with the inventory and generates DHCP
 * configuration files
 *
 */
class synd_plugin_dhcp extends Plugin {
	function __construct($config = null) {
		parent::__construct($config);
		SyndLib::attachHook('type', array($this, '_callback_type'));
		SyndLib::attachHook('types', array($this, '_callback_types'));
	}
	
	/**
	 * Instantiates the dhcpd.conf specific datatypes
	 * @access	protected
	 */
	function _callback_type(&$result, $type, $field) {
		switch ($type) {
			case 'DhcpUnsignedInteger32':
				$result = new DhcpDatatype(new SyndNumericDatatype($field), 'unsigned integer 32');
				return true;
			case 'DhcpText':
				$result = new DhcpDatatype(new SyndStringDatatype($field), 'text');
				return true;
			case 'DhcpIpAddress':
				$result = new DhcpDatatype(new SyndIpAddressDatatype($field), 'ip-address');
				return true;
		}
	}

	/**
	 * Adds the dhcpd.conf specific datatypes
	 * @access	protected
	 */
	function _callback_types(&$result) {
		$result['DhcpUnsignedInteger32'] = new DhcpDatatype(new SyndNumericDatatype(), 'unsigned integer 32');
		$result['DhcpText'] = new DhcpDatatype(new SyndStringDatatype(), 'text');
		$result['DhcpIpAddress'] = new DhcpDatatype(new SyndIpAddressDatatype(), 'ip-address');
	}

	/**
	 * Generates a DHCP config file given a list of networks
	 * 
	 * For example
	 *  wget http://www.example.com/synd/inventory/dhcp/dhcpd.conf?network[]=1.2.3.4:24&network[]=1.2.3.5:24 -o dhcpd.conf
	 */
	function _view_dhcpd_conf(Request $request, Template $page) {
		global $synd_maindb;
		require_once 'core/model/node/interface.class.inc';
		set_time_limit(300);
		
		$sql = "
			SELECT ii.node_id 
			FROM synd_instance i, synd_inv_interface ii, synd_inv_nic nn
			WHERE 
				i.node_id = nn.parent_node_id AND
				ii.nic_node_id = nn.node_id AND
				ii.info_ip_address IS NOT NULL AND
				nn.info_mac_address IS NOT NULL";
		
		if (!empty($_REQUEST['network'])) {
			$filters = array();
			foreach ((array)$_REQUEST['network'] as $network) {
				list ($ip, $bits) = explode(':', $network);
				if (null == ($encoded = synd_node_interface::ip2long($ip)) || !is_numeric($bits))
					throw new InvalidArgumentException();
				$mask = hexdec(dechex(0xFFFFFFFF << (32 - $bits)));
				$filters[] = $synd_maindb->bitand('ii.info_ip_encoded', $mask).' = '.hexdec(dechex($encoded & $mask));
			}
			$sql .= ' AND ('.implode(' OR ', $filters).')';
		}
		
		$sql .= "
			ORDER BY i.parent_node_id, ii.info_hostname";
		
		$storage = SyndNodeLib::getDefaultStorage('interface');
		$interfaces = $storage->getInstances($synd_maindb->getCol($sql));
		
		$page->assign('interfaces', $interfaces);
		
		@header('Content-Type: text/plain');
		return array('content' => $page->fetch('plugin/dhcp/dhcpd.conf.tpl'), 'page' => null);
	}
}

/**
 * Wraps a datatype to add dhcpd.conf functionality
 */
class DhcpDatatype implements ISyndDatatype {
	/**
	 * @access	protected
	 */
	var $_type = null;

	/**
	 * @access	protected
	 */
	var $_primitive = null;
	
	/**
	 * @param	ISyndType	The primitive type
	 * @param	string	Primitive dhcpd.conf type {unsigned integer 32, text, ip-address, ...}
	 */
	function DhcpDatatype($type, $primitive) {
		$this->_type = $type;
		$this->_primitive = $primitive;
	}
	
	function getDatatypeName() {
		return SyndLib::translate('DHCP '.$this->_type->getDatatypeName());
	}
	
	function isInheritedFrom($interface) {
		return $this instanceof $interface;
	}
	
	function toString() {
		return $this->_type->toString();
	}
	
	function validate($value) {
		return null == $value ? false : $this->_type->validate($value);
	}
	
	function modify($value) {
		return $this->_type->modify($value);
	}
	
	function quote($value) {
		return $this->_type->quote($value);
	}
	
	function getOptionSpace() {
		return false !== ($i = strpos($this->_type->toString(), '.')) ? substr($this->_type->toString(), 0, $i) : 0;
	}
	
	function getOptionCode() {
		static $spaces = array();
		$space = $this->getOptionSpace();
		$offset = 0 === $space ? 128 : 1;
		
		if (!isset($spaces[$space]))
			$spaces[$space] = array();
		else if (count($spaces[$space]) + $offset >= 254)
			return null;

		$key = $this->toString();
		if (!isset($spaces[$space][$key]))
			$spaces[$space][$key] = $offset + count($spaces[$space]);

		return $spaces[$space][$key];
	}
	
	function getPrimitiveType() {
		return $this->_primitive;
	}
}
