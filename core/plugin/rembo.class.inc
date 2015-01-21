<?php
require_once 'core/Plugin.class.inc';

/**
 * Rembo inventory plugin 
 *
 * Integates Rembo with the inventory and generates Rembo 
 * configuration files
 *
 * @link		http://www.rembo.com/
 */
class synd_plugin_rembo extends Plugin {
	function __construct($config = null) {
		parent::__construct($config);
		SyndLib::attachHook('type', array($this, '_callback_type'));
		SyndLib::attachHook('types', array($this, '_callback_types'));
	}
	
	/**
	 * Instantiates the Rembo specific datatypes
	 * @access	protected
	 */
	function _callback_type(&$result, $type, $field) {
		switch ($type) {
			case 'RemboField':
				$result = new RemboField(new SyndStringDatatype($field));
				return true;
			case 'RemboOptions':
				$result = new RemboOptions($field);
				return true;
		}
	}

	/**
	 * Adds the Rembo specific datatypes
	 * @access	protected
	 */
	function _callback_types(&$result) {
		$result['RemboField'] = new RemboField(new SyndStringDatatype());
		$result['RemboOptions'] = new RemboOptions();
	}

	/**
	 * Generates a Rembo config file given a list of networks
	 * 
	 * For example
	 *  wget http://www.example.com/synd/inventory/rembo/rembo.conf?network[]=1.2.3.4:24&network[]=1.2.3.5:24 -o rembo.conf
	 *
	 * @param	array	Request arguments
	 * @return	array
	 */
	function _view_rembo_conf(Request $request, Template $page) {
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
		return array('content' => $page->fetch('plugin/rembo/rembo.conf.tpl'), 'page' => null);
	}
}

/**
 * Wraps a datatype to add rembo.conf functionality
 */
class RemboField implements ISyndDatatype {
	/**
	 * @access	protected
	 */
	var $_type = null;

	/**
	 * @param	ISyndType	The primitive type
	 */
	function RemboField($type) {
		$this->_type = $type;
	}
	
	function getDatatypeName() {
		return SyndLib::translate('Rembo '.$this->_type->getDatatypeName());
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
}

/**
 * Wraps a datatype to add rembo.conf options functionality
 */
class RemboOptions extends SyndDatatype {
	function RemboOptions($field = null) {
		parent::SyndDatatype($field);
	}

	function isInheritedFrom($interface) {
		if ('RemboField' == $interface)
			return true;
		return parent::isInheritedFrom($interface);
	}
	
	function getDatatypeName() {
		return SyndLib::translate('Rembo Options');
	}
	
	function validate($value) {
		if (!preg_match('/^\s*(\w+\s*)+\s*$/', $value))
			return SyndLib::translate("Invalid Rembo-options '%s'", tpl_chop($value,20));
		return true;
	}

	function quote($value) {
		return preg_replace('/[^\w ]/', '', $value);
	}
}
