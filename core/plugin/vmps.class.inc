<?php
require_once 'core/Plugin.class.inc';

/**
 * Cisco VMPS inventory plugin 
 *
 * Integates VMPS with the inventory and generates VMPS
 * configuration files
 *
 */
class synd_plugin_vmps extends Plugin {
	/**
	 * Generates a VMPS config file given a list of networks
	 * 
	 * For example
	 *  wget http://www.example.com/synd/inventory/vmps/vmps.db?network[]=1.2.3.4:24&network[]=1.2.3.5:24 -o vmps.db
	 *
	 * @param	array	Request arguments
	 * @return	array
	 */
	function _view_vmps_db(Request $request, Template $page) {
		global $synd_maindb;
		require_once 'core/model/node/interface.class.inc';
		set_time_limit(300);
		
		$sql = "
			SELECT nn.INFO_MAC_ADDRESS, v.INFO_HEAD
			FROM synd_inv_vlan v, synd_inv_vlan_network n, synd_inv_interface ii, synd_inv_nic nn
			WHERE
				v.node_id = n.parent_node_id AND
				nn.node_id = ii.nic_node_id AND
				nn.info_mac_address IS NOT NULL AND
				".$synd_maindb->bitand('n.info_encoded_mask', 'ii.info_ip_encoded')." = n.info_encoded_net";
		
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
			ORDER BY v.info_head, nn.info_mac_address";
			
		$page->assign('interfaces', $synd_maindb->getAssoc($sql));
		
		@header('Content-Type: text/plain');
		return array('content' => $page->fetch('plugin/vmps/vmps.db.tpl'), 'page' => null);
	}
	
	/**
	 * Updates network statistics based on VMPS log data
	 * @param	string	VMPS log data to parse
	 * @return	bool	Return true on success, false on failure
	 */
	function log($log) {
		global $synd_maindb;
		require_once 'core/model/node/nic.class.inc';
		set_time_limit(3600);
		ignore_user_abort(false);
		
		$pattern = '/(\w+\s+\d{1,2})\s+(\d+:\d+:\d+)[^:]*:\s*ALLOW:\s*([\da-f]{12})\W+(\w+)\D+(\d{1,3}(?:\.\d{1,3}){3})\s+port\s+(\S+)/iS';
		$vlans = array();
		$log .= "\r\n";
		$synd_maindb->begin();
		
		for ($i=0, $j=0; false !== ($j = strpos($log, "\n", $j+1)); $i=$j+1) {
			if (preg_match($pattern, substr($log,$i,$j), $matches)) {
				$name = strtolower($matches[4]);
				$ts = strtotime($matches[1].date(" Y ").$matches[2]);
				
				if (!array_key_exists($name, $vlans)) {
					$sql = "
						SELECT node_id FROM synd_inv_vlan
						WHERE LOWER(info_head) =".$synd_maindb->quote($name);

					// Autocreate non-existing VLANs
					if (null == ($vlans[$name] = $synd_maindb->getOne($sql)) && 
						null !== ($inventory = Module::getInstance('inventory')) &&
						null !== ($folder = $inventory->getAgentFolder())) {
						$vlan = $folder->appendChild($folder->_storage->factory('vlan'));
						$vlan->data['INFO_HEAD'] = $matches[4];
						$vlan->save();
						$vlan->flush();
						$vlans[$name] = $vlan->nodeId;
					}
				}

				$sql = "
					UPDATE synd_inv_nic
					SET 
						info_last_seen = $ts,
						info_last_switch = ".$synd_maindb->quote($matches[5]).",
						info_last_switch_port = ".$synd_maindb->quote($matches[6]).",
						info_last_vlan = ".$synd_maindb->quote($vlans[$name])."
					WHERE 
						info_mac_address = ". $synd_maindb->quote(synd_node_nic::fixMacAddress($matches[3]))." AND
						(info_last_seen IS NULL OR info_last_seen < $ts)";
				$result = $synd_maindb->query($sql);
				
				if (SyndLib::isError($result)) {
					$synd_maindb->rollback();
					trigger_error($result->getMessage(), E_USER_WARNING);
					return false;
				}
				
				if ($i > 0 && 0 == $i%1000) {
					$synd_maindb->commit();
					$synd_maindb->begin();
				}
			}
		}
		
		$synd_maindb->commit();
		return true;
	}

	/**
	 * VMPS log target
	 * 
	 * Updates interface statistics based on a POST'ed fragment from 
	 * the VMPS log and
	 *
	 * For example
	 *  cat vmps.log | /usr/bin/curl -f -s -k -H "Content-Type: text/plain" --data-binary @- http://www.example.com/synd/inventory/vmps/vmps.log
	 *
	 * @param	array	Request arguments
	 * @return	array
	 */
	function _view_vmps_log(Request $request, Template $page) {
		global $HTTP_RAW_POST_DATA;
		if ($this->log($HTTP_RAW_POST_DATA))
			return array('content' => '', 'page' => null);
		throw new RuntimeException();
	}
}
