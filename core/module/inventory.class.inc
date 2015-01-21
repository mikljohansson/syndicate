<?php
require_once 'core/module/node.class.inc';
require_once 'core/model/node/issue.class.inc';
require_once 'core/model/node/class.class.inc';
require_once 'core/Plugin.class.inc';

/**
 * Inventory and configuration management database (CMDB) module
 *
 * The inventory handles items such as computers and other equipment 
 * and issues relating to the repair of them. Base point of access 
 * for the inventory agents.
 *
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_inventory extends synd_module_node {
	function __construct($config) {
		parent::__construct($config);
		SyndLib::attachHook('index_active_clsids', array($this, '_callback_index_active_clsids'));
		SyndLib::attachHook('index_security_attributes', array($this, '_callback_index_security_attributes'));
		SyndLib::attachHook('user_summary_default', array($this, '_callback_user_summary_default'), 25);
		SyndLib::attachHook('user_summary_result', array($this, '_callback_user_summary_result'));
		SyndLib::attachHook('search_costcenter', array($this, '_callback_search_costcenter'));
		SyndLib::attachHook('menu', array($this, '_callback_menu'));
		SyndLib::attachHook('menu_admin', array($this, '_callback_menu_admin'));
	}

	/**
	 * Loads the issue.css style sheets
	 */
	function request(Request $request) {
		global $synd_config;
		if ($this->isPermitted('read')) {
			if (!empty($path))
				SyndLib::attachHook('breadcrumbs', array($this, '_callback_breadcrumbs'));
			SyndLib::attachHook('block', array($this, '_callback_block'));
		}

		tpl_load_stylesheet(tpl_design_uri('module/inventory/inventory.css'));
		tpl_load_stylesheet(tpl_design_uri('module/issue/issue.css'));
		tpl_load_script(tpl_design_uri('js/ole.js'));

		// Translate /issue/123/ style uris
		if (isset($path[0]) && is_numeric($path[0]) && null != ($issue = synd_node_issue::findIssue(SyndNodeLib::getDefaultStorage('issue'), $path[0])))
			return parent::request(array_merge(array('view',$issue->nodeId), array_slice($path, 1)), $request);

		return parent::request($request);
	}
	
	function _callback_menu(&$result, $menu) {
		if ($this->isPermitted('read'))
			$menu->append(tpl_link('inventory'), SyndLib::translate('Inventory'), '', array($this, '_callback_menu_default'), array('inventory','inventory/*'));
	}
	
	function _callback_menu_admin(&$result, $menu) {
		if ($this->isPermitted('admin'))
			$menu->append(tpl_link('system','inventory','admin'), SyndLib::translate('Inventory'));
	}

	function _callback_menu_default($menu) {
		$menu->append(tpl_link('inventory','items'), SyndLib::translate('List items'));
		$menu->append(tpl_link('inventory','leases'), SyndLib::translate('List leases'));
		$menu->append(tpl_link('inventory','search'), SyndLib::translate('Search'));
	}

	function _callback_menu_commands($menu) {
		$menu->append(tpl_link('inventory','newItem'), SyndLib::translate('Add item'),
			SyndLib::translate('Create simple inventory item without any specific configuration'));
		$menu->append(tpl_link('inventory','newDevice'), SyndLib::translate('Add device'), 
			SyndLib::translate('Create configurable device typically those having network connectivity'));
		$menu->append("javascript:synd_ole_call('".rawurlencode(tpl_view_call('inventory','move'))."');", 
			SyndLib::translate('Move to folder'));
		$menu->append("javascript:synd_ole_call('".rawurlencode(tpl_view_call('inventory','newIssue'))."');", 
			SyndLib::translate('Create issue'), SyndLib::translate('Create an issue for the selected assets'));
		$menu->append("javascript:synd_ole_call('".rawurlencode(tpl_view_call('inventory','newInvoice'))."');", 
			SyndLib::translate('Create invoice'));
		$menu->append("javascript:synd_ole_call('".rawurlencode(tpl_view_call('inventory','report','print'))."');", 
			SyndLib::translate('Print receipts'), SyndLib::translate('Print receipts for invoices, repairs and leases'));
		$menu->append("javascript:synd_ole_call('".rawurlencode(tpl_view_call('inventory','return'))."');", 
			SyndLib::translate('Return items'));
	}

	function _callback_menu_reports($menu) {
		$menu->append("javascript:synd_ole_call('".rawurlencode(tpl_view('inventory','report'))."details.xls', true);", 
			SyndLib::translate('Details (Excel)'));
		$menu->append("javascript:synd_ole_call('".rawurlencode(tpl_view('inventory','report'))."invoice.xls', true);", 
			SyndLib::translate('Invoice (Excel)'));
		$menu->append("javascript:synd_ole_call('".rawurlencode(tpl_view('inventory','report'))."counts.xls', true);", 
			SyndLib::translate('Statistics (Excel)'), SyndLib::translate('Statistics per model showing counts, number of repairs and time spent.'));
		$menu->append("javascript:synd_ole_call('".rawurlencode(tpl_view('inventory','report'))."clients.xls', true);", 
			SyndLib::translate('Clients (Excel)'), SyndLib::translate('Client information including email, phone and address.'));
	}

	function _callback_index_active_clsids(&$result) {
		$result[] = 'item';
		$result[] = 'lease';
	}

	function _callback_index_security_attributes(&$result, $user) {
		foreach (array_keys($folders = $this->getFolders()) as $key) {
			if ($folders[$key]->hasPermission($user[0], 'read')) {
				foreach ($folders[$key]->getSecurityAttributes() as $attribute)
					$result[] = $attribute;
			}
		}
	}

	function _callback_breadcrumbs(&$result, Template $page) {
		$result[] = array(
			'uri' => tpl_link('inventory'), 
			'text' => SyndLib::translate('Inventory'));
		
		if (null !== $this->_node && $this->_node->isPermitted('read')) {
			$crumbs = array();
			$parent = $this->_node;
			while (null != ($parent = $parent->getParent()) && !$parent->isNull()) {
				$crumbs[] = array(
					'uri' => tpl_link($parent->getHandler(),'view',$parent->nodeId),
					'text' => $parent->toString());
			}
			$result = array_merge($result, array_reverse($crumbs));
		}
	}
	
	function _callback_block(&$result, Template $page) {
		require_once 'design/gui/MenuBuilder.class.inc';
		$block = clone $page;
		$block->assign('module', $this);

		$menu = new ListMenu();
		$menu->append(null, SyndLib::translate('Commands'), '', array($this, '_callback_menu_commands'));
		$menu->append(null, SyndLib::translate('Reports'), '', array($this, '_callback_menu_reports'));

		$result[] = array(
			'text' => SyndLib::translate('Inventory'),
			'content' => $block->fetch(tpl_design_path('module/inventory/blocks/default.tpl'), array('menu'=>$menu)));

		// Add folders block
		if (count($folders = SyndLib::filter($this->getFolders(),'isPermitted','read'))) {
			$folder = null;
			if (null != $this->_node) {
				if (($this->_node instanceof synd_node_folder))
					$folder = $this->_node;
				else if (($this->_node->getParent() instanceof synd_node_folder))
					$folder = $this->_node->getParent();
			}

			$block->assign('folder', $folder);
			$block->assign('folders', $folders);
			$block->assign('expand', null != $folder ? $folder->getBranch() : array());

			$result[] = array(
				'text' => SyndLib::translate('Folders'),
				'content' => $block->fetch(tpl_design_path('module/inventory/blocks/folders.tpl')));
		}
	}

	function _callback_user_summary_default(&$result, Request $request, Template $page, User $user) {
		if ($this->isPermitted('read')) {
			SyndNodeLib::loadClass('item');
			$items = synd_node_item::getClientItems($user);
			$items = SyndLib::filter($items, 'isPermitted', 'read');

			// Find client leases
			$storage = SyndNodeLib::getDefaultStorage('lease');
			$database = $storage->getDatabase();

			$sql = "
				SELECT l.node_id FROM synd_inv_lease l
				WHERE 
					l.client_node_id = ".$database->quote($user->nodeId)." AND
					l.ts_terminated IS NULL";
			$leases = $storage->getInstances($database->getCol($sql));
			$leases = SyndLib::filter($leases, 'isPermitted', 'read');

			$sql = "
				SELECT COUNT(1) FROM synd_inv_lease l
				WHERE 
					l.client_node_id = ".$database->quote($user->nodeId)." AND
					l.ts_terminated IS NOT NULL";
			$prevLeasesCount = $database->getOne($sql);

			$summary = clone $page;
			$summary->assign('module', $this);
			$summary->assign('user', $user);

			$summary->assign('items', $items);
			$summary->assign('leases', $leases);
			$summary->assign('descriptions', $this->getServiceLevelDescriptions($user));
			$summary->assign('prevLeasesCount', $prevLeasesCount);

			$result .= $summary->fetch(tpl_design_path('module/inventory/user_summary_default.tpl'));
		}
	}

	/**
	 * Displays links to current SLAs
	 */
	function _callback_user_summary_result(&$result, Request $request, Template $page, User $user) {
		if ($this->isPermitted('read') && count($descriptions = $this->getServiceLevelDescriptions($user))) {
			$summary = clone $page;
			$summary->assign('descriptions', $descriptions);
			$result[] = $summary->fetch(tpl_design_path('module/inventory/user_summary_result.tpl'));
		}
	}
	
	function _callback_search_costcenter(&$result, $query) {
		$result[] = SyndType::factory('search_collection', 'costcenter', $query);	
	}

	function getDescription() {
		return SyndLib::translate('Inventory GUI and agent endpoint module.');
	}

	/**
	 * @access	protected
	 */
	function _hook_manage_permissions(&$result) {
		$result[] = $this;	
	}

	/**
	 * @access	protected
	 */
	function _hook_defined_events(&$result, $node) {
		if (($node[0] instanceof synd_node_project)) 
			$result['onnonwarranty'] = SyndLib::translate('When an non warranty issue comes in');
	}
	
	/**
	 * @access	protected
	 */
	function _hook_folder_content_types(&$result) {
		$result[] = 'item';
		$result[] = 'computer';
		$result[] = 'lease';
		$result[] = 'file';
		$result[] = 'costcenter';
		$result[] = 'sld';
		$result[] = 'license';
		$result[] = 'vlan';
	}
	
	function _hook_field_extension(&$result, $extension) {
		$extension[0]->addField('cost');
		$extension[0]->addField('location');
		$extension[0]->addFieldAlias('creator', 'client');
	}

	function _hook_field_extension_description(&$result, $field) {
		switch ($field) { 
			case 'cost':
				$result .= ' '.SyndLib::translate('Defines the monetary cost of an inventory item.');
				break;
			case 'location':
				$result .= ' '.SyndLib::translate('The physical location of an inventory item.');
				break;
			case 'client':
				$result .= ' '.SyndLib::translate('Defines the user of an inventory item.');
				break;
		}
	}
	
	/**
	 * Updates a device according to an XML specification
	 * @link	http://svn.synd.info/synd/branches/php4/core/module/inventory/agent/device.dtd
	 * @param	string	XML device info (received from agent)
	 * @return	synd_node_computer
	 */
	function agent($xml) {
		require_once 'core/model/node/os.class.inc';
		require_once 'core/model/SyndXMLNode.class.inc';
		
		// Write to logfile
		SyndLib::file_put_contents('/tmp/agent-'.$_SERVER['SERVER_NAME'].'.log', $xml);
		
		$document = SyndXMLNode::parse($xml);
		$computer = $this->_findComputerInstance($document);
		$computer->save();
		
		$listener = $document->getElement('listener');
		$computer->setRemoteTransport(
			$listener->getAttribute('method'), $listener->getContent(), 
			$listener->getAttribute('action'), $listener->getAttribute('version'));
		
		$motherboard = $document->getElement('motherboard');
		$bios = $motherboard->getElement('bios');
		$computer->setMotherBoard(
			$motherboard->getAttribute('vendor'), 
			$bios->getAttribute('version'), 
			$motherboard->getAttribute('serial'));

		$rams = $motherboard->getElements('ram');
		$computer->setPhysicalMemory(array_sum(SyndLib::invoke($rams, 'getAttribute', 'size')));
		
		$cpu = $document->getElement('cpu');
		$computer->setProcessor(
			$cpu->getAttribute('version'), $cpu->getAttribute('frequency'), 
			count($document->getElements('cpu')), $cpu->getAttribute('serial'));
			
		$keyboard = $document->getElement('keyboard');
		if (!$keyboard->isNull())
			$computer->setKeyboard($keyboard->getContent());
			
		foreach (array_keys($disks = $document->getElements('disk')) as $key) {
			$computer->addDiskDrive(
				$disks[$key]->getAttribute('version'), $disks[$key]->getAttribute('size'), 
				$disks[$key]->getAttribute('cache'), $disks[$key]->getAttribute('name'));
		}
		
		foreach (array_keys($roms = $document->getElements('rom')) as $key)
			$computer->addROMDrive($roms[$key]->getAttribute('version'), $roms[$key]->getAttribute('mountpoint'));

		// Add network interface cards
		$macs = SyndLib::invoke($computer->getNetworkInterfaces(), 'getMacAddress');
		foreach (array_keys($nics = $document->getElements('nic')) as $key) {
			if (!in_array($nics[$key]->getAttribute('mac'), $macs)) {
				// Check for existing MAC address
				$sql = "
					SELECT n.parent_node_id FROM synd_inv_nic n
					WHERE n.info_mac_address = ".$computer->_db->quote(strtoupper(trim($nics[$key]->getAttribute('mac'))));
				if (null !== ($pid = $computer->_db->getOne($sql)) && $pid != $computer->nodeId)
					continue;
				
				$nic = $computer->appendChild($computer->_storage->factory('nic'));
				$nic->setMacAddress($nics[$key]->getAttribute('mac'));
				$nic->setDescription($nics[$key]->getAttribute('version'));
				$nic->save();

				$macs[] = $nics[$key]->getAttribute('mac');
				
				// Add network interfaces to new computers
				if ($computer->isNew()) {
					$config = $computer->getConfig();
					if ($config->isNew())
						$config->save();
					
					foreach (array_keys($interfaces = $nics[$key]->getElements('interface')) as $key2) {
						$interface = $config->appendChild($config->_storage->factory('interface'));
						if (null != $interfaces[$key2]->getAttribute('ip')) {
							$interface->setIpAddress($interfaces[$key2]->getAttribute('ip'));
							$interface->setHostname(gethostbyaddr($interfaces[$key2]->getAttribute('ip')));
						}

						$interface->setNetworkInterface($nic);
						$interface->save();
					}
				}
			}
		}

		foreach (array_keys($videocards = $document->getElements('videocard')) as $key) {
			$computer->addVideoController(
				$videocards[$key]->getAttribute('version'), $videocards[$key]->getAttribute('ram'),
				$videocards[$key]->getAttribute('hres'), $videocards[$key]->getAttribute('vres'),
				$videocards[$key]->getAttribute('bits'), $videocards[$key]->getAttribute('frequency'));
		}

		foreach (array_keys($soundcards = $document->getElements('soundcard')) as $key)
			$computer->addSoundDevice($soundcards[$key]->getAttribute('version'));
	
		foreach (array_keys($monitors = $document->getElements('monitor')) as $key) {
			$computer->addMonitor(
				$monitors[$key]->getAttribute('vendor'), 
				$monitors[$key]->getAttribute('version'), 
				$monitors[$key]->getAttribute('size'));
		}

		// Add operating systems
		foreach (array_keys($oses = $document->getElements('os')) as $key) {
			if (null != $oses[$key]->getAttribute('name') && 
				null !== ($os = synd_node_os::findIdentifier($computer, $oses[$key]->getAttribute('name')))) {
				$os->setVersion($oses[$key]->getAttribute('version'), $oses[$key]->getAttribute('release'));
				$os->setMachineName($oses[$key]->getAttribute('machinename'));
			
				$image = $oses[$key]->getElement('image');
				$os->setLoadedImage($image->getContent());
				
				// Add installed software
				$result = array();
				foreach (array_keys($products = $oses[$key]->getElements('product')) as $key2) {
					$result[] = array(
						$products[$key2]->getAttribute('version'), $products[$key2]->getAttribute('release'),
						null === $products[$key2]->getAttribute('state') ? 'ok' : ($products[$key2]->getAttribute('state') ? 'ok' : 'failed'));
				}
				
				$os->setSoftware($result);
				$os->save();
			}
		}
		
		return $computer;
	}

	/**
	 * Inventory agent RPC callback
	 * @see		agent()
	 * @return	bool
	 */
	function _public_agent($xml) {
		set_time_limit(300);
		return null !== $this->agent($xml);
	}
	
	/**
	 * Inventory agent callback
	 * @see		agent()
	 */
	function _view_agent(Request $request, Template $page) {
		global $HTTP_RAW_POST_DATA;
		if (empty($HTTP_RAW_POST_DATA)) {
			trigger_error('No message supplied', E_USER_WARNING);
			throw new InvalidArgumentException();
		}
		
		set_time_limit(300);
		$this->agent($HTTP_RAW_POST_DATA);
		return array('content' => '200', 'page' => null);
	}
	
	/**
	 * @access	private
	 */
	function _findComputerInstance($document) {
		$storage = SyndNodeLib::getDefaultStorage('computer');
		$database = $storage->getDatabase();
		
		// Find existing computer instance
		$computer = null;
		if (count($cpuids = array_filter(SyndLib::invoke($document->getElements('cpu'), 'getAttribute', 'serial')))) {
			$sql = "
				SELECT i.node_id FROM synd_instance i, synd_inv_computer c
				WHERE 
					i.node_id = c.node_id AND
					c.info_cpu_id IN (".implode(',',$database->quote($cpuids)).")
				ORDER BY i.ts_update DESC";
			$computer = $storage->getInstance($database->getOne($sql));
		}
		
		if (null === $computer && count($macs = array_filter(SyndLib::invoke($document->getElements('nic'), 'getAttribute', 'mac')))) {
			$sql = "
				SELECT i.node_id FROM synd_instance i, synd_inv_computer c, synd_inv_nic n
				WHERE 
					i.node_id = c.node_id AND
					c.node_id = n.parent_node_id AND
					n.info_mac_address IN (".implode(',',$database->quote($macs)).")
				ORDER BY i.ts_update DESC";
			$computer = $storage->getInstance($database->getOne($sql));
		}

		if (null === $computer && count($serials = array_filter(SyndLib::invoke($document->getElements('motherboard'), 'getAttribute', 'serial')))) {
			$sql = "
				SELECT i.node_id FROM synd_instance i, synd_inv_computer c
				WHERE 
					i.node_id = c.node_id AND
					c.info_mb_serial IN (".implode(',',$database->quote($serials)).")
				ORDER BY i.ts_update DESC";
			$computer = $storage->getInstance($database->getOne($sql));
		}		
		
		if (null === $computer && null !== ($folder = $this->getAgentFolder()))
			$computer = $folder->appendChild($storage->factory('computer'));
			
		return $computer;
	}
	
	/**
	 * @deprecated
	 */
	function _public_findComputerInstance($version, $ids) {
		return '';
	}
	
	function _view_index(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		$page->assign('title', SyndLib::translate('Inventory'));
		return array('content' => $page->fetch('module/inventory/index.tpl'));
	}
	
	function _view_admin(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		$page->assign('title', SyndLib::translate('Inventory administration'));
		$page->assign('request', $request);
		return array('content' => $page->fetch('module/inventory/admin.tpl'));
	}

	function _view_search(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();

		if (empty($request['query'])) 
			$page->assign('title', SyndLib::translate('Search'));
		else
			$page->assign('title', SyndLib::translate('Search: %s', tpl_chop(strip_tags($request['query']),20)));

		$sections = array(
			'all' => SyndLib::translate('All'),
			'lease' => SyndLib::translate('Leases'),
			'item' => SyndLib::translate('Items'));
		
		$section = isset($request['section']) ? $request['section'] : 'all';
		$classes = SyndLib::sort($this->getClasses());
		$sections = array_merge($sections, SyndLib::array_prepend(
			SyndLib::invoke($classes,'toString'), ' - '));

		$page->assign('request', $request);
		$page->assign('section', $section);
		$page->assign('sections', $sections);

		$mres = array();
		$page->assign('mres', $mres);
		
		$content = $page->fetch(tpl_design_path('module/inventory/search.tpl'));
		return isset($mres['redirect']) ? $mres  : array('content' => $content);
	}
	
	/**
	 * Searches for items
	 * 
	 * Returns an array with matching items and the total count of
	 * matching items as array(array(object synd_node_item), integer count)
	 *
	 * @param	string	Query string
	 * @param	integer	Result offset
	 * @param	integer	Result limit
	 * @param	string	Specific class to search for (see synd_node_class)
	 * @param	array	Fields to search numerically on
	 * @param	string	Default operator in {&&, ||}
	 * @return	Collection
	 */
	function _search($query, $offset, $limit, $clsid = null, $fields = null, $operator = '&&') {
		require_once 'core/db/SyndDBLib.class.inc';

		$storage = SyndNodeLib::getDefaultStorage('item');
		$database = $storage->getDatabase();
		
		$alias = DatabaseConnection::alias('synd_instance');
		$alias2 = DatabaseConnection::alias('synd_inv_item');

		$from  = array("
			synd_inv_item $alias2, synd_instance $alias
				LEFT JOIN synd_class_values v 
				ON ($alias.node_id = v.node_id)
				LEFT JOIN synd_relation r
				ON ($alias.node_id = r.child_node_id)");
		$where = array("$alias.node_id = $alias2.node_id");

		$equery = call_user_func(array(SyndNodeLib::loadClass('item'),'getEntityQuery'), $storage);
		$columns = array('v.value', 'r.parent_node_id');
		foreach ($equery->getTables() as $table)
			$columns = array_merge($columns, SyndLib::array_prepend($database->getTableStructure($table)->getColumnNames(), DatabaseConnection::alias($table).'.'));

		// Add search filters
		if (null != ($sql = SyndDBLib::sqlLikeExpr($this->searchPreprocess($query), $columns, '&&' == $operator ? 'AND' : 'OR')))
			$where[] = $sql;
			
		// Add class filters
		if (null != $clsid) {
			$where[] = "$alias.class_node_id = '$clsid'";
			foreach ((array)$fields as $id => $expression) {
				if (null != ($sql = SyndDBLib::parseNumeric($expression, "k$id.value"))) {
					$from[] = "synd_class_values k$id";
					$where[] = "k$id.node_id = $alias.node_id AND k$id.field_id = $id AND ($sql)";
					$enable = true;
				}
			}
		}

		// Instantiate collection
		if (empty($from) || empty($where)) 
			$result = SyndType::factory('null_collection');
		else {
			$columns = array("$alias.node_id");
			if (count($order = tpl_sort_order('item'))) {
				foreach ($order as $column) {
					if (!is_numeric($column) && !empty($column)) 
						$columns[] = $column;
				}
			}

			$sql = "
				SELECT DISTINCT ".implode(', ', array_unique($columns))."
				FROM ".implode(', ', $from)."
				WHERE ".implode(' AND ', $where);
				
			if (count($order))
				$sql .= "\nORDER BY ".tpl_sort_sql($order);
			$result = SyndType::factory('query_collection', $storage, $sql);
		}
		
		return $result;
	}
	
	function _getCollection($request) {
		if (empty($request['collections']) || !count($ids = array_unique((array)$request['collections'])))
			$result = null;
		else if (1 == count($ids))
			$result = SyndLib::getInstance(reset($ids));
		else {
			$result = SyndType::factory('composite_collection');
			foreach ($ids as $id) {
				if (null !== ($collection = SyndLib::getInstance($id)))
					$result->appendCollection($collection);
			}
		}
		return $result;
	}

	function _view_leases(Request $request, Template $page) {
		global $synd_user;
		require_once 'core/db/SyndDBLib.class.inc';
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();

		$page->assign('title', SyndLib::translate('Leasing report'));

		if (isset($request['output'])) {
			require_once 'core/module/inventory/LeaseReport.class.inc';

			$storage = SyndNodeLib::getDefaultStorage('lease');
			$database = $storage->getDatabase();
			
			$query = $database->createQuery();
			$l = $query->join('synd_inv_lease', 'l');

			if (!empty($request['query']) && null != ($textFilter = SyndDBLib::sqlLikeExpr($request['query'], array("$l.info_body"))))
				$query->where($textFilter);
			if (empty($request['empty'])) {
				$in = $query->join('synd_instance', 'in');
				$it = $query->join('synd_inv_item', 'it');
				$query->where("$in.node_id = $it.node_id");

				$u = $query->join('synd_inv_used', 'u');
				$query->where("$l.node_id = $u.parent_node_id");
				$query->where("$in.node_id = $u.child_node_id");
				$query->where("$u.ts_expire IS NULL");

				if (!empty($textFilter))
					$query->where(SyndDBLib::sqlLikeExpr($request['query'], array("$it.info_make", "$it.info_model")));
			}

			if (!empty($request['folder']) && null !== ($folder = SyndNodeLib::getInstance($request['folder'])) && $folder->isPermitted('read')) {
				if (empty($request['recurse']))
					$query->where("$l.parent_node_id = ".$database->quote($folder->nodeId));
				else {
					$tree = SyndLib::filter($folder->getFolderTree(), 'isPermitted', 'read');
					$query->where("$l.parent_node_id IN (".implode(', ', 
						$database->quote(SyndLib::collect($tree,'nodeId'))).")");
				}
			}
			else {
				$query->where("$l.parent_node_id IN (".implode(', ', $database->quote(
					SyndLib::collect($this->getFolderTree($synd_user,'read'),'nodeId'),"'")).")");
			}

			if (isset($request['state'])) {
				switch ($request['state']) {
					case 'terminated':
						$query->where("$l.ts_terminated IS NOT NULL");
						break;
					case 'active':
						$query->where("$l.ts_terminated IS NULL");
						break;
				}
			}

			// Created between
			if (!empty($request['created'][0]) && -1 != ($ts = strtotime($request['created'][0])))
				$query->where("$l.ts_create >= $ts");
			if (!empty($request['created'][1]) && -1 != ($ts = strtotime($request['created'][1])))
				$query->where("$l.ts_create <= $ts");

			// Expires between
			if (!empty($request['expires'][0]) && -1 != ($ts = strtotime($request['expires'][0])))
				$query->where("$l.ts_expire >= $ts");
			if (!empty($request['expires'][1]) && -1 != ($ts = strtotime($request['expires'][1])))
				$query->where("$l.ts_expire <= $ts");
				
			if (!empty($request['costcenter'])) {
				if (null !== ($costcenter = SyndLib::runHook('find_costcenter', $request['costcenter']))) {
					$query->where("$l.costcenter_node_id = ".$database->quote($costcenter->id()));
					$page->assign('costcenter', $costcenter->getLogin());
				}
				else {
					$query->where("$l.costcenter_node_id = ".$database->quote('node.user_case.'.$request['costcenter']));
					$page->assign('costcenter', $request['costcenter']);
				}
			}

			if (!empty($request['project'])) {
				if (null !== ($project = SyndLib::runHook('find_project', $request['project']))) {
					$query->where("$l.project_node_id = ".$database->quote($project->id()));
					$page->assign('project', $project->getLogin());
				}
				else {
					$query->where("$l.project_node_id = ".$database->quote('node.user_case.'.$request['project']));
					$page->assign('project', $request['project']);
				}
			}

			if (!empty($request['customer']) && 
				null !== ($module = Module::getInstance('user')) &&
				null !== ($user = $module->resolveEmail($request['customer']))) {
				$query->where("$l.client_node_id = ".$database->quote($user->nodeId));
				$page->assign('customer', null != $user->getEmail() ? $user->getEmail() : $user->getLogin());
			}

			if (!isset($request['report'])) 
				$request['report'] = 'list';
			if (!isset($request['output']))
				$request['output']['html'] = '';
			
			switch ($request['report']) {
				default:
					$report = new LeaseListingReport($storage, $query, tpl_sort_order('lease'));
					$format = 'listing';
					$name = 'Leases';
			}
			
			$page->assign('report', $report);
			if (isset($request['output']['xls'])) {
				SyndLib::runHook('setlocale', LC_NUMERIC);
				header('Content-Type: application/xls');
				header('Content-Disposition: attachment; filename="'.SyndLib::translate("$name %s.xls", date('Y-m-d')));
				return array('content' => $page->fetch(tpl_design_path("module/inventory/report/leases/formats/$format.xls.tpl")), 'page' => null);
			}

			$page->assign('format', $format);
		}
		
		$page->assign('request', $request);
		$page->assign('folders', $this->getFolders());
		return array('content' => $page->fetch(tpl_design_path('module/inventory/report/leases/index.tpl')));
	}

	function _view_items(Request $request, Template $page) {
		global $synd_user;
		require_once 'core/db/SyndDBLib.class.inc';
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();

		$page->assign('title', SyndLib::translate('Items report'));

		if (isset($request['output'])) {
			require_once 'core/module/inventory/ItemReport.class.inc';

			$storage = SyndNodeLib::getDefaultStorage('lease');
			$database = $storage->getDatabase();
			
			$query = $database->createQuery();
			$i = $query->join('synd_instance', 'i');
			$ii = $query->join('synd_inv_item', 'ii');
			$query->where("$i.node_id = $ii.node_id");

			if (!empty($request['query']) && null != ($textFilter = SyndDBLib::sqlLikeExpr($request['query'], array("$ii.info_make","$ii.info_model"))))
				$query->where($textFilter);

			if (!empty($request['folders']))
				$folders = SyndLib::filter($storage->getInstances($request['folders']), 'isPermitted', 'read');
			else
				$folders = $this->getFolderTree($synd_user,'read');
				
			if (!empty($folders) && empty($request['recurse']))
				$selfolders = SyndLib::collect($folders, 'nodeId');
			else {
				$selfolders = array();
				foreach (array_keys($folders) as $key) {
					foreach (array_keys($tree = $folders[$key]->getFolderTree('read')) as $key2)
						$selfolders[] = $tree[$key2]->nodeId;
				}
			}

			$query->where("$i.parent_node_id IN (".implode(',',$database->quote($selfolders)).")");
			if (!empty($request['folders']))
				$page->assign('selfolders', $selfolders);
			
			if (!empty($request['class']) && null != ($class = SyndNodeLib::getInstance($request['class']))) {
				$query->where("$i.class_node_id = ".$database->quote($class->nodeId));
				$page->assign('class', $class);
			}

			if (!empty($request['costcenter'])) {
				if (null !== ($costcenter = SyndLib::runHook('find_costcenter', $request['costcenter']))) {
					$query->where("costcenter_node_id = ".$database->quote($costcenter->id()));
					$page->assign('costcenter', $costcenter->getLogin());
				}
				else {
					$query->where("costcenter_node_id = ".$database->quote('node.user_case.'.$request['costcenter']));
					$page->assign('costcenter', $request['costcenter']);
				}
			}

			if (!empty($request['project'])) {
				if (null !== ($project = SyndLib::runHook('find_project', $request['project']))) {
					$query->where("project_node_id = ".$database->quote($project->id()));
					$page->assign('project', $project->getLogin());
				}
				else {
					$query->where("project_node_id = ".$database->quote('node.user_case.'.$request['project']));
					$page->assign('project', $request['project']);
				}
			}

			// Active customer
			if (!empty($request['customer']) && 
				null !== ($module = Module::getInstance('user')) &&
				null !== ($user = $module->resolveEmail($request['customer']))) {
				$u = $query->join('synd_inv_used', 'u');
				$l = $query->join('synd_inv_lease', 'l');
				$query->where("$i.node_id = $u.child_node_id");
				$query->where("$l.node_id = $u.parent_node_id");
				$query->where("$l.client_node_id = ".$database->quote($user->nodeId));
				$query->where("$u.ts_expire IS NULL");
				$page->assign('customer', null != $user->getEmail() ? $user->getEmail() : $user->getLogin());
			}

			// Updated between
			if (!empty($request['updated'][0]) && -1 != ($ts = strtotime($request['updated'][0])))
				$query->where("$i.ts_update >= $ts");
			if (!empty($request['updated'][1]) && -1 != ($ts = strtotime($request['updated'][1])))
				$query->where("$i.ts_update <= $ts");

			if (!isset($request['report'])) 
				$request['report'] = 'list';
			if (!isset($request['output']))
				$request['output']['html'] = '';
			
			if (empty($request['date']) || -1 == ($ts = strtotime($request['date'])))
				$ts = time();
			
			switch ($request['report']) {
				case 'trends':
					$report = new UsageTrendsReport($storage, $query, $ts, new UsageTrendsModelGrouping());
					$format = 'trends';
					$name = 'Availability';
					break;
				default:
					$report = new ItemListingReport($storage, $query, tpl_sort_order('item'));
					$format = 'listing';
					$name = 'Items';
			}
			
			$page->assign('report', $report);
			if (isset($request['output']['xls'])) {
				SyndLib::runHook('setlocale', LC_NUMERIC);
				header('Content-Type: application/xls');
				header('Content-Disposition: attachment; filename="'.SyndLib::translate("$name %s.xls", date('Y-m-d')));
				return array('content' => $page->fetch(tpl_design_path("module/inventory/report/items/formats/$format.xls.tpl")), 'page' => null);
			}

			$page->assign('format', $format);
		}
		
		$page->assign('request', $request);
		$page->assign('folders', $this->getFolders());
		$page->assign('classes', $this->getClasses());
		return array('content' => $page->fetch(tpl_design_path('module/inventory/report/items/index.tpl')));
	}
	
	function _view_report(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		if (!isset($request[0]) || !preg_match('/^\w+(\.\w{3})?$/', $request[0]))
			throw new ForbiddenException();
		if (null == ($report = tpl_design_path("module/inventory/report/{$request[0]}.tpl")))
			throw new NotFoundException();
		
		set_time_limit(600);
		$page->assign('title', SyndLib::translate('Report'));

		$page->assign('request', $request);
		$page->assign('collection', $this->_getCollection($request));

		$mres = array('content' => $page->fetch($report));
		
		if (false !== strpos($request[0], '.xls')) {
			header('Content-Type: application/xls');
			$mres['page'] = null;
		}
		
		return $mres;
	}

	function _view_move(Request $request, Template $page) {
		global $synd_user;
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		if (null == ($collection = $this->_getCollection($request)))
			return null;
		$items = $collection->getFilteredContents(array('synd_node_item'));
		if (false !== array_search(false, SyndLib::invoke($items, 'isPermitted', 'write')))
			throw new ForbiddenException();
		
		$page->assign('title', SyndLib::translate('Move items to folder'));
		$page->assign('items', $items);
		$page->assign('module', $this);
		$page->assign('collection', $collection);
		
		if (isset($request['confirm']) && null !== ($folder = SyndNodeLib::getInstance($request['folder']))) {
			foreach (array_keys($items) as $key) {
				$items[$key]->setFolder($folder);
				$items[$key]->save();
			}
			
			if (!empty($request['receipt']) && 
				null !== ($receipt = SyndNodeLib::getInstance($request['receipt'])) &&
				null !== ($printer = $this->getPrinter())) {
				require_once 'core/lib/SyndPrint.class.inc';

				$filter = array(
					'DATE'			=> date('Y-m-d H:i'),
					'ASSIGNED'		=> array(
						'CAPTION'		=> null != $synd_user->getContact() ? $synd_user->toString().' ('.$synd_user->getContact().')' : $synd_user->toString(),
						'NAME'			=> $synd_user->toString(),
						'PHONE'			=> $synd_user->getPhone()),
					'FOLDER'		=> $folder->toString(),
					'ITEM'			=> array(
						'COUNT'			=> count($items)));
				
				foreach (array_keys($items) as $key) {
					$filter['ITEM'][] = array(
						'CAPTION'	=> $items[$key]->toString(),
						'SERIAL'	=> $items[$key]->getSerial());
				}
				
				header('Content-Type: application/pdf');
				return array('content' => SyndPrint::pdfMerge($receipt->path(), $filter), 'page' => null);
				
				SyndPrint::pdfPrint($printer, $receipt->path(), $filter);
				$page->assign('printer', $printer);
				$page->assign('receipt', $receipt);
			}
			
			$page->assign('folder', $folder);
			return array('content' => $page->fetch(tpl_design_path('module/inventory/flow/move/receipt.tpl')));
		}
		else {
			return array('content' => $page->fetch(
				tpl_design_path('module/inventory/flow/move/confirm.tpl')));
		}
	}

	function _view_newItem(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		$node = SyndNodeLib::factory('item');
		return array('redirect' => tpl_view('inventory','edit',$node->nodeId));
	}

	function _view_newDevice(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		$node = SyndNodeLib::factory('computer');
		return array('redirect' => tpl_view('inventory','edit',$node->nodeId));
	}

	function _view_newCategory(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		$node = SyndNodeLib::factory('class');
		return array('redirect' => tpl_view_jump('inventory','edit',$node->nodeId));
	}
	
	function _view_newFolder(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		$node = SyndNodeLib::factory('folder');
		$node->setParent(SyndNodeLib::getInstance('null.inventory'));
		return array('redirect' => tpl_view_jump('inventory','edit',$node->nodeId));
	}
	
	function _view_newLease(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		if (null === ($user = SyndNodeLib::getInstance($request[0])))
			throw new NotFoundException();
		$node = SyndNodeLib::factory('lease');
		$node->setCustomer($user);
		return array('redirect' => tpl_view_jump('inventory','edit',$node->nodeId));
	}
	
	function _view_newLeases(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();

		if (null == ($prototype = SyndNodeLib::getInstance($request['prototype']))) {
			$page->assign('node', SyndNodeLib::factory('lease'));
			return array('content' => $page->fetch(tpl_design_path('module/inventory/flow/newLeases/step1.tpl')));
		}
		else if (isset($request['data'])) {
			if (count($errors = $prototype->validate($request['data']))) {
				$page->assign('node', $prototype);
				$page->assign('errors', $errors);
				return array('content' => $page->fetch(tpl_design_path('module/inventory/flow/newLeases/step1.tpl')));
			}
			else {
				$prototype->merge($request['data']);
				return array('redirect' => tpl_view('inventory','newLeases',array('prototype' => $prototype->nodeId)));
			}
		}
		else if (isset($request['clients'])) {
			$clients = SyndNodeLib::getInstances($request['clients']);
			$leases = array();
			
			foreach (array_keys($clients) as $key) {
				$lease = $prototype->copy();
				$lease->setCustomer($clients[$key]);
				$lease->save();
				$leases[] = $lease;
			}
			
			$page->assign('leases', $leases);
			return array('content' => $page->fetch(tpl_design_path('module/inventory/flow/newLeases/step4.tpl')));
		}
		else if (isset($request['query'])) {
			$module = Module::getInstance('user');
			$collection = $module->findUsers(preg_replace('/(?<=\w)\s+(?=\w)/', ' || ', $request['query']));
			$clients = $collection->getContents(0, 250);
			
			$page->assign('prototype', $prototype);
			$page->assign('clients', $clients);
			return array('content' => $page->fetch(tpl_design_path('module/inventory/flow/newLeases/step3.tpl')));
		}
		else {
			$page->assign('prototype', $prototype);
			return array('content' => $page->fetch(tpl_design_path('module/inventory/flow/newLeases/step2.tpl')));
		}
	}
	
	function _view_newLeasing(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		if (!empty($request['lease_node_id']) && null !== ($lease = SyndNodeLib::getInstance($request['lease_node_id'])) && !$lease->isPermitted('write') ||
			!empty($request['item_node_id']) && null !== ($item = SyndNodeLib::getInstance($request['item_node_id'])) && !$item->isPermitted('write'))
			throw new ForbiddenException();
		
		$page->assign('request', $request);

		if (!empty($lease))
			$page->assign('lease', $lease);
		if (!empty($item))
			$page->assign('item', $item);

		if (!empty($lease) && !empty($item)) {
			if (isset($request['confirm'])) {
				if (null !== ($leasing = $item->setCustomer($lease)))
					$lease->appendChild($leasing);
				if (null !== $this->getLeasedFolder())
					$item->setFolder($this->getLeasedFolder());
				$item->save();
				return array('content' => $page->fetch(tpl_design_path('module/inventory/flow/newLeasing/step3.tpl')));
			}

			return array('content' => $page->fetch(tpl_design_path('module/inventory/flow/newLeasing/step2.tpl')));
		}

		if (!empty($request['lease']))
			$leases = SyndLib::filter($this->findLeases($request['lease']),'isPermitted','write');
		else if (!empty($lease))
			$leases = array($lease);
		else
			$leases = array();
		
		if (!empty($request['item']))
			$items = SyndLib::filter($this->findItems($request['item']),'isPermitted','write');
		else if (!empty($item))
			$items = array($item);

		if (empty($lease) && isset($leases) && 1 == count($leases))
			$page->assign('lease', $leases[key($leases)]);
		if (isset($leases))
			$page->assign('leases', $leases);
			
		if (empty($item) && isset($items) && 1 == count($items))
			$page->assign('item', $items[key($items)]);
		if (isset($items))
			$page->assign('items', $items);
		
		$page->assign('title', SyndLib::translate('Hand out item on a lease'));
		return array('content' => $page->fetch(
			tpl_design_path('module/inventory/flow/newLeasing/step1.tpl')));
	}

	function _view_newIssue(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		if (null == ($collection = $this->_getCollection($request)))
			return null;
		$items = $collection->getFilteredContents(array('synd_node_item'));
		if (false !== array_search(false, SyndLib::invoke($items, 'isPermitted', 'read')))
			throw new ForbiddenException();

		$issue = SyndNodeLib::factory('repair');
		foreach (array_keys($items) as $key)
			$issue->appendChild($items[$key]);
		return array('redirect' => tpl_view_jump($issue->getHandler(),'edit',$issue->nodeId));
	}

	function _view_newInvoice(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		if (null == ($collection = $this->_getCollection($request)))
			return null;

		if (null == ($project = $this->getInvoiceProject()))
			return array('content' => $page->fetch(tpl_design_path('module/inventory/flow/newInvoice/error_config_project.tpl')));
			
		if (empty($request['prototype']) || null == ($prototype = SyndNodeLib::getInstance($request['prototype'])))
			$prototype = $this->getInvoiceProject()->appendChild(SyndNodeLib::factory('invoice'));
		if (!$prototype->isPermitted('write'))
			throw new ForbiddenException();

		$args = array(
			'prototype' => $prototype->nodeId, 
			'collections' => array($collection->id()));
		$errors = null;

		// Handle submission
		if (!isset($request['data'])) 
			$data = $prototype->data;
		else {
			if (!count($errors = $prototype->validate($request['data']))) {
				$prototype->merge($request['data']);
				if (!isset($request[0]) || 3 != $request[0]) {
					$args = array_merge(array('inventory','newInvoice', min(3, isset($request[0]) ? $request[0]+1 : 2)), array($args));
					return array('redirect' => call_user_func_array('tpl_view_jump', $args));
				}
			}

			$data = array_merge($prototype->data, $request['data']);
		}

		// Create invoices
		if (isset($request[0]) && 2 == $request[0] && !empty($request['post'])) {
			set_time_limit(1800);
			$count = 0;
			
			foreach ($collection->getFilteredContents(array('synd_node_lease')) as $lease) {
				$invoice = clone $prototype;
				$invoice->setLease($lease);
				$invoice->save();
				$invoice->flush();
				$count++;
			}
			
			$args['count'] = $count;
			return array('redirect' => tpl_view_jump('inventory','newInvoice',
				array_merge(array(min(3, isset($request[0]) ? $request[0]+1 : 2)), $args)));
		}
		
		$page->append('tabs', array(
			'uri' => tpl_link_jump('inventory','newInvoice',$args),
			'text' => SyndLib::translate('Specification'),
			'template' => array($prototype,'flow/step1.tpl'),
			'selected' => empty($request[0])));

		$page->append('tabs', array(
			'uri' => tpl_link_jump('inventory','newInvoice',array_merge(array(2),$args)),
			'text' => SyndLib::translate('Confirm'),
			'template' => array($prototype,'flow/step2.tpl'),
			'selected' => isset($request[0]) && 2 == $request[0]));

		$page->append('tabs', array(
			'uri' => tpl_link_jump('inventory','newInvoice',array_merge(array(3),$args)),
			'text' => SyndLib::translate('Result'),
			'template' => array($prototype,'flow/step3.tpl'),
			'selected' => isset($request[0]) && 3 == $request[0]));
		
		return array('content' => $page->fetch(tpl_design_path('gui/pane/flow.tpl'),array(
			'collection' => $collection, 'data' => $data, 
			'errors' => $errors, 'request' => $request)));
	}
	
	function _view_payInvoices(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();

		$page->assign('request', $request);

		$errors = array();
		$invoices = array();

		if (isset($request['invoices'])) {
			SyndNodeLib::loadClass('invoice');
			$payments = $this->_parseInvoicePayments($request['invoices']);
			$collection = synd_node_invoice::resolveInvoiceNumbers(
				SyndLib::array_collect($payments,'number'));
			
			if (null != $collection) {
				foreach (array_keys($contents = $collection->getFilteredContents(array('synd_node_invoice'))) as $key)
					$invoices[$contents[$key]->getInvoiceNumber()] = $contents[$key];

				$seenNumbers = array();
				foreach ($payments as $payment) {
					if (!isset($invoices[$payment['number']])) {
						$errors[$payment['number']]['msg'] = SyndLib::translate('No invoice for payment %s found (%.00f paid)', 
							$payment['number'], $payment['amount']);
					}
					else if (!$invoices[$payment['number']]->isPermitted('write')) {
						$errors[]['msg'] = SyndLib::translate("You do not have write access to invoice '%s', please contact your systems administrator", 
							$invoices[$payment['number']]->getInvoiceNumber());
						unset($invoices[$payment['number']]);
					}
					else if (in_array($payment['number'], $seenNumbers)) {
						$errors[]['msg'] = SyndLib::translate('Invoice %s is paid twice (%.00f paid)', 
							$page->fetchnode($invoices[$payment['number']],'number.tpl'), $payment['amount']);
					}
					else if ($invoices[$payment['number']]->isPaid()) {
						$errors[$payment['number']]['msg'] = SyndLib::translate('Invoice %s is already marked as paid (%.00f paid)', 
							$page->fetchnode($invoices[$payment['number']],'number.tpl'), $payment['amount']);
					}
					else if ((float)$invoices[$payment['number']]->getAmount() != (float)$payment['amount']) {
						$errors[$payment['number']]['msg'] = SyndLib::translate('Incorrect amount paid for %s (%.00f paid, should be %.00f)', 
							$page->fetchnode($invoices[$payment['number']],'number.tpl'), 
							$payment['amount'], $invoices[$payment['number']]->getAmount());
					}

					$seenNumbers[] = $payment['number'];
				}
			}

			$page->assign('invoices', $invoices);
			$page->assign('errors', $errors);
			
			if (isset($request['confirm']) && empty($errors)) {
				foreach ($payments as $payment) {
					$invoices[$payment['number']]->setPaid($payment['date']);
					if (!empty($request['comment'])) {
						$task = $invoices[$payment['number']]->appendChild(SyndNodeLib::factory('task'));
						$content = $task->getContent();
						$content->setContent($request['comment']);
						$task->save();
					}
				}

				return array('content' => $page->fetch(tpl_design_path('module/inventory/flow/payInvoices/step2.tpl')));
			}
		}
		
		return array('content' => $page->fetch(tpl_design_path('module/inventory/flow/payInvoices/step1.tpl')));
	}
	
	/**
	 * Parses invoice numbers 
	 * @link	http://www.plusgirot.se/sitemod/upload/root/www_plusgirot_se/pdfer/allmanbeskrivning/G446_Postbeskrivning_Inbetalningsservice.pdf
	 */
	function _parseInvoicePayments($buffer) {
		$payments = array();
		$timestamp = null;

		foreach (explode("\n", $buffer) as $line) {
			if (preg_match('/^\s*(\d+)\s*,\s*(\d+)\s*,?\s*([\d\-]*)\s*$/', $line, $matches)) {
				$payments[] = array(
					'number' => $matches[1],
					'amount' => $matches[2],
					'date' => empty($matches[3]) || -1 == strtotime($matches[3]) ? time() : strtotime($matches[3]));
			}
			else {
				switch (substr(trim($line),0,2)) {
					case '00':
						if (-1 == ($timestamp = strtotime(substr($line,50,6))))
							$timestamp = null;
						break;
					case '40':
						if (null == ($number = ltrim(substr($line,2,25))))
							continue;
						if (null == ($amount = round(ltrim(substr($line,27,13),'0')/100,2)))
							continue;
						$payments[] = array(
							'number' => $number,
							'amount' => $amount,
							'date' => $timestamp);
						break;
				}
			}
		}
		
		return $payments;
	}
	
	/**
	 * Handles the return of leased items
	 */
	function _view_return(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		if (null == ($collection = $this->_getCollection($request)) ||
			!count($items = $collection->getFilteredContents(array('synd_node_item'))))
			return null;
		if (false !== array_search(false, SyndLib::invoke($items, 'isPermitted','write')))
			throw new ForbiddenException();
		
		if (null !== ($project = $this->getRepairProject())) {
			$issue = $project->appendChild($project->_storage->factory('terminate'));
			$issue->setCustomer($items[key($items)]->getCustomer());
			$issue->setTitle(SyndLib::translate("Return items '%s'", tpl_chop(implode(', ',SyndLib::invoke($items,'getSerial')),40)));
			foreach (array_keys($items) as $key)
				$issue->appendChild($items[$key]);
			return array('redirect' => tpl_view_jump($issue->getHandler(),'edit',$issue->nodeId));
		}
	}
	
	/**
	 * Batch import items from CSV
	 */
	function _view_importItems(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();

		set_time_limit(3600);
		$storage = SyndNodeLib::getDefaultStorage('item');
		$page->assign('request', $request);
		$page->assign('folders', SyndLib::filter($this->getFolders(),'isPermitted','write'));
		$page->assign('classes', SyndLib::filter($this->getClasses(),'isPermitted','read'));

		$errors = array();
		$items = array();
		
		$fieldOptions = array(
			'INFO_MAKE'				=> new SyndStringDatatype($page->text('Make')),
			'INFO_MODEL'			=> new SyndStringDatatype($page->text('Model')),
			'INFO_SERIAL_MAKER' 	=> new SyndStringDatatype($page->text('Maker S/N')),
			'INFO_SERIAL_INTERNAL'	=> new SyndStringDatatype($page->text('Serial')),
			'installation'			=> new SyndStringDatatype($page->text('Installation ID')),
			'TS_DELIVERY'			=> new SyndStringDatatype($page->text('Delivered')),
			);

		if (empty($request['folder']) || null == ($folder = $storage->getInstance($request['folder']))) 
			$errors['folder'] = $page->text('Please select a folder to place imported items');
		else if (!$folder->isPermitted('write'))
			throw new ForbiddenException();
		else
			$page->assign('folder', $folder);

		if (empty($request['class']) || null == ($class = $storage->getInstance($request['class']))) 
			$errors['class'] = $page->text('Please select an item category to create');
		else if (!$class->isPermitted('read'))
			throw new ForbiddenException();
		else {
			$page->assign('class', $class);
			foreach ($class->getFields() as $key => $field)
				$fieldOptions[$key] = $field;
		}
		
		$fieldids = isset($request['fields']) ? array_filter($request['fields'],'strlen') : array();
		$fields = SyndLib::array_kintersect($fieldOptions, array_flip($fieldids));
		
		$page->assign('fieldOptions', SyndLib::array_kdiff($fieldOptions, array_flip($fieldids)));
		$page->assign('fields', $fields);

		if (isset($request['confirm'])) {
			if (empty($fields)) 
				$errors[] = $page->text('Please select one or more fields to import');
			if (empty($errors))
				$rows = $this->_parseCSV($page, $fields, $request['csv'], $errors);
			
			if (empty($errors)) {
				$database = $storage->getDatabase();
				$database->begin();
				
				try {
					foreach ($rows as $rowIndex => $data) {
						$item = $storage->factory('item');
						$item->setFolder($folder);
						$item->setClass($class);
						
						$err = $item->validate($data);
						
						if (empty($err)) {
							$item->merge($data);
							$items[] = $item;
						}
						else {
							foreach ($err as $error)
								$errors[] = $page->text("Row %d: %s", $rowIndex+1, $error);
						}
						
						if (count($errors) > 10) {
							$errors[] = $page->text('...');
							break;
						}
					}
					
					if (empty($errors)) {
						foreach ($items as $item)
							$item->save();
						$storage->flush();
						$database->commit();
					}
					else {
						$database->rollback();
					}
				}
				catch (Exception $e) {
					$database->rollback();
					throw $e;
				}
			}

			$page->assign('items', $items);
			$page->assign('errors', $errors);
			
			if (isset($request['confirm']) && empty($errors)) {
				return array('content' => $page->fetch(tpl_design_path('module/inventory/flow/importItems/step2.tpl')));
			}
		}
		
		return array('content' => $page->fetch(tpl_design_path('module/inventory/flow/importItems/step1.tpl')));
	}
	
	
	/**
	 * Parses item CSV date
	 */
	function _parseCSV(Template $page, $fields, $buffer, &$errors) {
		$rows = array();
		$maxRows = 500;

		foreach (explode("\n", $buffer) as $rowIndex => $line) {
			$line = trim($line);
			
			if ($line) {
				$values = preg_split('/\s*[,;\t]\s*/', $line);
				$data = array();
				$i = 0;
				
				foreach ($fields as $key => $field) {
					if ($i >= count($values)) {
						$errors[] = $page->text("Row %d:\tMissing required values ('%s')", $rowIndex+1, tpl_chop($line,20));
						continue 2;
					}
					else if (is_object($field) && true !== ($message = $field->validate($values[$i]))) {
						$errors[] = $page->text("Row %d:\t%s", $rowIndex+1, $message);
						continue 2;
					}
					
					if (is_numeric($key)) {
						if (!isset($data['values']))
							$data['values'] = array();
						$data['values'][$key] = $values[$i];
					}
					else {
						$data[$key] = $values[$i];
					}
					
					$i++;
				}
				
				if ($i+1 < count($values)) {
					$errors[] = $page->text("Row %d:\tToo many values. Expected %d fields, found %d", $rowIndex+1, count($data), count($values));
				}
				else {
					$rows[] = $data;
				}
			}
			
			if (count($errors) > 10) {
				$errors[] = $page->text('...');
				break;
			}
			
			if (count($rows) > $maxRows) {
				$errors[] = $page->text('Too many rows, please import a maximum of %d at a time', $maxRows);
				break;
			}
		}
		
		return $rows;
	}

	/**
	 * Batch import items from CSV
	 */
	function _view_importLeases(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();

		set_time_limit(3600);
		$storage = SyndNodeLib::getDefaultStorage('item');
		$page->assign('request', $request);
		$page->assign('folders', SyndLib::filter($this->getFolders(),'isPermitted','write'));

		$errors = array();
		$nodes = array();
		
		$fieldOptions = array(
			'customer'			=> new SyndStringDatatype($page->text('Username')),
			'serial'			=> new SyndStringDatatype($page->text('Item S/N')),
			'created'			=> new SyndDateDatatype($page->text('Valid From')),
			'expires'			=> new SyndDateDatatype($page->text('Valid To')),
			'terminated'		=> new SyndDateDatatype($page->text('Terminated')),
			);

		if (empty($request['folder']) || null == ($folder = $storage->getInstance($request['folder']))) 
			$errors['folder'] = $page->text('Please select a folder to place imported items');
		else if (!$folder->isPermitted('write'))
			throw new ForbiddenException();
		else
			$page->assign('folder', $folder);

		$fieldids = isset($request['fields']) ? array_filter($request['fields'],'strlen') : array();
		$fields = SyndLib::array_kintersect($fieldOptions, array_flip($fieldids));
		
		$page->assign('fieldOptions', SyndLib::array_kdiff($fieldOptions, array_flip($fieldids)));
		$page->assign('fields', $fields);

		if (isset($request['confirm'])) {
			if (empty($fields)) 
				$errors[] = $page->text('Please select one or more fields to import');
			if (empty($errors))
				$rows = $this->_parseCSV($page, $fields, $request['csv'], $errors);

			if (empty($errors)) {
				// Find items
				$ids = array();
				foreach ($rows as $rowIndex => $data) {
					if (isset($data['serial']))
						$ids[$rowIndex] = strtoupper($data['serial']);
				}

				$items = array();
				$found = synd_node_item::findSerialNumbers($storage, $ids);

				if (empty($errors)) {
					foreach ($found as $item) {
						$rowIndex = array_search($item->data['INFO_SERIAL_MAKER'], $ids);
						if ($rowIndex === false)
							$rowIndex = array_search($item->data['INFO_SERIAL_INTERNAL'], $ids);

						if (!$item->isPermitted('write')) {
							$errors[] = $page->text("Row %d: You lack write permission to item %s %s", 
								$rowIndex+1, $item->data['INFO_SERIAL_MAKER'], $item->data['INFO_SERIAL_INTERNAL']);
						}
						else if (!isset($request['reassign']) && !$item->getCustomer()->isNull()) {
							$errors[] = $page->text("Row %d: Item %s %s is already assigned to %s", 
								$rowIndex+1, $item->data['INFO_SERIAL_MAKER'], $item->data['INFO_SERIAL_INTERNAL'], 
								$page->fetchnode($item->getCustomer(),'head_view.tpl'));
						}
						else {
							if ($item->data['INFO_SERIAL_INTERNAL'] != null)
								$items[strtoupper($item->data['INFO_SERIAL_INTERNAL'])] = $item;
							if ($item->data['INFO_SERIAL_MAKER'] != null)
								$items[strtoupper($item->data['INFO_SERIAL_MAKER'])] = $item;
						}

						if (count($errors) > 10) {
							$errors[] = $page->text('...');
							break;
						}
					}
				}

				if (empty($errors) && count($found) != count($ids)) {
					foreach (SyndLib::array_kdiff(array_flip($ids), $items) as $serial => $rowIndex) {
						$errors[] = $page->text("Row %d:\tNo item matching serial number '%s' found", $rowIndex+1, tpl_chop($serial,20));

						if (count($errors) > 10) {
							$errors[] = $page->text('...');
							break;
						}
					}
				}

			}
			
			if (empty($errors)) {
				$database = $storage->getDatabase();
				$database->begin();
				
				try {
					foreach ($rows as $rowIndex => $data) {
						$lease = $storage->factory('lease');
						$lease->setFolder($folder);
						
						$err = $lease->validate($data);
						if (empty($err))
							$lease->merge($data);
						else {
							foreach ($err as $error)
								$errors[] = $page->text("Row %d: %s", $rowIndex+1, $error);
						}
						
						if (empty($errors) && isset($ids[$rowIndex], $items[$ids[$rowIndex]]))
							$items[$ids[$rowIndex]]->setCustomer($lease);
						
						if (empty($errors))
							$nodes[] = $lease;
						
						if (count($errors) > 10) {
							$errors[] = $page->text('...');
							break;
						}
					}
					
					if (empty($errors)) {
						foreach ($nodes as $lease)
							$lease->save();
						foreach ($items as $item)
							$item->save();
						
						$storage->flush();
						$database->commit();
					}
					else {
						$database->rollback();
					}
				}
				catch (Exception $e) {
					$database->rollback();
					throw $e;
				}
			}

			$page->assign('nodes', $nodes);
			$page->assign('errors', $errors);
			
			if (isset($request['confirm']) && empty($errors)) {
				return array('content' => $page->fetch(tpl_design_path('module/inventory/flow/importLeases/step2.tpl')));
			}
		}
		
		return array('content' => $page->fetch(tpl_design_path('module/inventory/flow/importLeases/step1.tpl')));
	}

	/**
	 * Search for leases
	 * @param	string	Search string
	 * @return	array
	 */
	function findLeases($query) {
		if (null != ($collection = SyndLib::runHook('search', 'lease', $this->searchPreprocess($query), null, false)))
			return $collection->getContents(0, 15);
		return array();
	}

	/**
	 * Search for items
	 * @param	string	Search string
	 * @return	array
	 */
	function findItems($query) {
		if (null != ($collection = SyndLib::runHook('search', 'item', $this->searchPreprocess($query), null, false)))
			return $collection->getContents(0, 15);
		return array();
	}

	/**
	 * @access	protected
	 */
	function extractItems($children) {
		$items = array();
		foreach (array_keys($children) as $key) {
			if (($children[$key] instanceof synd_node_item))
				$items[$children[$key]->nodeId] = $children[$key];
			else if (($children[$key] instanceof synd_node_lease) || ($children[$key] instanceof synd_node_repair)) {
				foreach (array_keys($it = $children[$key]->getItems()) as $key2)
					$items[$it[$key2]->nodeId] = $it[$key2];
			}
		}
		return $items;
	}

	/**
	 * Returns the selected printer
	 * @return	string
	 */
	function getPrinter() {
		if (isset($_SESSION['synd']['public']['inventory']['printer']))
			return $_SESSION['synd']['public']['inventory']['printer'];
		return SyndLib::reset($this->getPrinters());
	}

	/**
	 * Returns all configured printers
	 * @return	array
	 */
	function getPrinters() {
		if (is_array($printers = SyndLib::getVar('inventory_printers')))
			return array_combine($printers, $printers);
		return array();
	}

	/**
	 * Returns top-level foldesr
	 * @return	array
	 */
	function getFolders() {	
		profiler_enter(__METHOD__);

		$storage = SyndNodeLib::getDefaultStorage('project');
		$database = $storage->getDatabase();
		$sql = "
			SELECT i.node_id FROM synd_instance i, synd_inv_folder f
			WHERE
				i.node_id = f.node_id AND
				i.parent_node_id = 'null.inventory'";
		$folders = $storage->getInstances($database->getCol($sql));
		
		profiler_leave(__METHOD__);
		return $folders;
	}

	/**
	 * Returns recursive folder tree
	 * @param	User	User to get tree for
	 * @param	string		Permission to check
	 * @return	array
	 */
	function getFolderTree($client, $perm = 'read') {
		profiler_enter(__METHOD__);
		$storage = SyndNodeLib::getDefaultStorage('folder');
		$database = $storage->getDatabase();
		
		// Top-level folders
		$sql = "
			SELECT i.node_id FROM synd_instance i, synd_inv_folder f
			WHERE
				i.node_id = f.node_id AND
				i.parent_node_id = 'null.inventory'";
		$roots = $storage->getInstances($database->getCol($sql));
		$folders = array();

		foreach (array_keys($roots) as $key) {
			if ($roots[$key]->hasPermission($client, $perm)) {
				foreach (array_keys($tree = $roots[$key]->getFolderTree()) as $key2) {
					if ($tree[$key2]->hasPermission($client, $perm))
						$folders[$key2] = $tree[$key2];
				}
			}
		}

		profiler_leave(__METHOD__);
		return $folders;
	}
	
	function getProjects() {
		$storage = SyndNodeLib::getDefaultStorage('project');
		$sql = "
			SELECT p.node_id FROM synd_project p
			WHERE p.parent_node_id = 'null.issue'";
		return SyndLib::filter($storage->getInstances($storage->getDatabase()->getCol($sql)), 'isPermitted', 'read');
	}
	
	function getClasses() {
		$storage = SyndNodeLib::getDefaultStorage('class');
		$sql = "SELECT c.node_id FROM synd_class c";
		return $storage->getInstances($storage->getDatabase()->getCol($sql));
	}
	
	/**
	 * Returns the list of current service level descriptions for a given user
	 *
	 * Agreements are inherited down the users organizational tree
	 *
	 * @param	User	User to fetch descriptions for
	 * @return	array
	 */
	function getServiceLevelDescriptions($user) {
		$branch = array($user->nodeId);
		for ($parent = $user->getOrganization(); !$parent->isNull(); $parent = $parent->getParent())
			$branch[] = $parent->nodeId;
		
		$storage = SyndNodeLib::getDefaultStorage('sld');
		$database = $storage->getDatabase();

		$sql = "
			SELECT DISTINCT ls.sld_node_id 
			FROM synd_inv_lease l, synd_inv_lease_sld ls
			WHERE 
				l.node_id = ls.lease_node_id AND
				(l.ts_expire IS NULL OR l.ts_expire >= ".time().") AND
				l.ts_terminated IS NULL AND ls.ts_terminated IS NULL AND
				l.client_node_id IN (".implode(',',$database->quote($branch)).")";
		return SyndLib::filter($storage->getInstances($database->getCol($sql)), 'isPermitted', 'read');
	}

	/**
	 * The project that gets assigned repair issues
	 * @return	synd_node_project
	 */
	function getRepairProject() {
		return SyndNodeLib::getInstance(SyndLib::getVar('inventory_repair_project'));
	}

	/**
	 * The project that gets assigned invoice issues
	 * @return	synd_node_project
	 */
	function getInvoiceProject() {
		return SyndNodeLib::getInstance(SyndLib::getVar('inventory_invoice_project'));
	}

	/**
	 * The folder for replaced items (when waiting for repair)
	 * @return	synd_node_folder
	 */
	function getRepairFolder() {
		return SyndNodeLib::getInstance(SyndLib::getVar('inventory_repair_folder'));
	}
	
	/**
	 * The folder for repaired items (after being repaired)
	 * @return	synd_node_folder
	 */
	function getRepairedFolder() {
		return SyndNodeLib::getInstance(SyndLib::getVar('inventory_repaired_folder'));
	}

	/**
	 * The folder for leased items (after being handed out on a lease)
	 * @return	synd_node_folder
	 */
	function getLeasedFolder() {
		return SyndNodeLib::getInstance(SyndLib::getVar('inventory_leased_folder'));
	}

	/**
	 * The folder for items whose lease is terminated
	 * @return	synd_node_folder
	 */
	function getTerminateFolder() {
		return SyndNodeLib::getInstance(SyndLib::getVar('inventory_terminate_folder'));
	}
	
	/**
	 * The folder for lease which is terminated
	 * @return	synd_node_folder
	 */
	function getTerminatedLeaseFolder() {
		return SyndNodeLib::getInstance(SyndLib::getVar('inventory_terminated_lease_folder'));
	}

	/**
	 * The folder for new items the agent finds
	 * @return	synd_node_folder
	 */
	function getAgentFolder() {
		return SyndNodeLib::getInstance(SyndLib::getVar('inventory_agent_folder'));
	}
	
	/**
	 * Default folder for leases
	 * @return	synd_node_folder
	 */
	function getLeaseFolder() {
		return SyndNodeLib::getInstance(SyndLib::getVar('inventory_lease_folder'));
	}
	
	/**
	 * Folder for sold items
	 * @return	synd_node_folder
	 */
	function getSoldFolder() {
		return SyndNodeLib::getInstance(SyndLib::getVar('inventory_sold_folder'));
	}
	
	/**
	 * @access	private
	 */
	function _view_setFolders(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		
		SyndLib::setVar('inventory_repair_project', $request['repair_project']);
		SyndLib::setVar('inventory_invoice_project', $request['invoice_project']);
		SyndLib::setVar('inventory_leased_folder', $request['leased_folder']);
		SyndLib::setVar('inventory_repair_folder', $request['repair_folder']);
		SyndLib::setVar('inventory_repaired_folder', $request['repaired_folder']);
		SyndLib::setVar('inventory_terminate_folder', $request['terminate_folder']);
		SyndLib::setVar('inventory_terminated_lease_folder', $request['terminated_lease_folder']);
		SyndLib::setVar('inventory_agent_folder', $request['agent_folder']);
		SyndLib::setVar('inventory_lease_folder', $request['lease_folder']);
		SyndLib::setVar('inventory_sold_folder', $request['sold_folder']);
	}

	/**
	 * @access	private
	 */
	function _view_addPrinter(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		if (!isset($request['printer']) || !$request['printer'])
			return null;
		
		$printers = $this->getPrinters();
		$printers[] = $request['printer'];
		SyndLib::setVar('inventory_printers', array_unique($printers));
	}
	
	/**
	 * @access	private
	 */
	function _view_delPrinter(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		if (!isset($request['printer']) || !$request['printer'])
			return null;

		$printers = $this->getPrinters();
		if (false !== ($key = array_search($request['printer'], $printers))) {
			unset($printers[$key]);
			SyndLib::setVar('inventory_printers', array_unique($printers));
		}
	}
	
	/**
	 * Applies the regexp preprocessor directives to a search string
	 * @param	string	Search string to process
	 * @param	string
	 */
	function searchPreprocess($string) {
		foreach ($this->getSearchDirectives() as $directive)
			$string = preg_replace($directive['pattern'], $directive['replacement'], $string);
		return $string;
	}
	
	/**
	 * @access	protected
	 * @return	array
	 */
	function getSearchDirectives() {
		if (null != ($patterns = SyndLib::getVar('inventory_search_patterns')))
			return $patterns;
		return array();
	}
	
	/**
	 * @access	private
	 */
	function _view_addTemplate(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		if (empty($request['template']) || empty($request['file']['tmp_name']))
			return null;
		if (!is_uploaded_file($request['file']['tmp_name']))
			throw new ForbiddenException();
		
		$file = SyndNodeLib::factory('file');
		$file->setParent(SyndNodeLib::getInstance('case.inventory.templates.'.$request['template']));
		$file->import($request['file']['tmp_name'], $request['file']['name']);
		$file->save();
	}

	/**
	 * Returns the list of templates
	 * @param	string	The id to fetch templates for
	 * @return	array	Returns a list of synd_node_file
	 */
	function getTemplates($id) {
		$storage = SyndNodeLib::getDefaultStorage('file');
		$database = $storage->getDatabase();
		
		$sql = "
			SELECT n.node_id FROM synd_node n, synd_node_file f
			WHERE
				n.node_id = f.node_id AND
				n.parent_node_id = ".$database->quote('case.inventory.templates.'.$id);
		return $storage->getInstances($database->getCol($sql));
	}
	
	/**
	 * Returns the list of templates
	 * @param	string	The class to fetch templates for
	 * @return	array	Returns a list of synd_node_file
	 */
	function getInheritedTemplates($class) {
		SyndNodeLib::loadClass(substr($class,10));
		
		$templates = array();
		$storage = SyndNodeLib::getDefaultStorage('file');
		$database = $storage->getDatabase();
		
		for ($class=strtolower($class); null != $class && 'syndnode' != $class; $class = strtolower(get_parent_class($class)))
			$templates = SyndLib::array_merge_assoc($templates, $this->getTemplates($class));
		
		return $templates;
	}

	/**
	 * @access	private
	 */
	function _view_addSearchPattern(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		if (empty($request['pattern']))
			return null;
		
		// Strip the 'e' (execute result as php code) modifier
		$pattern = preg_replace('/\/(\w*)e(\w*)$/', '/\1\2', $request['pattern']);
		
		$directives = $this->getSearchDirectives();
		$directives[] = array(
			'pattern' => $pattern, 
			'replacement' => $request['replacement']);
		
		SyndLib::setVar('inventory_search_patterns', $directives);
	}
	
	/**
	 * @access	private
	 */
	function _view_delSearchPattern(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();

		$directives = $this->getSearchDirectives();
		unset($directives[$request['key']]);
		SyndLib::setVar('inventory_search_patterns', $directives);
	}

	/**
	 * Returns non-terminated leases due to expire within a given time period
	 * @param	integer	Start of interval
	 * @param	integer	End of interval
	 * @return	array
	 */
	function _public_getExpiringLeases($tsStart, $tsStop) {
		if (!$this->isPermitted('read'))
			return false;
		$storage = SyndNodeLib::getDefaultStorage('lease');
		$database = $storage->getDatabase();
		$sql = "
			SELECT l.node_id FROM synd_inv_lease l
			WHERE 
				l.ts_terminated IS NULL AND
				l.ts_expire BETWEEN ".$database->quote($tsStart)." AND ".$database->quote($tsStop);
		return SyndLib::filter($storage->getInstances($database->getCol($sql)), 'isPermitted', 'read');
	}

	/**
	 * Returns leases terminated within a given time period
	 * @param	integer	Start of interval
	 * @param	integer	End of interval
	 * @return	array
	 */
	function _public_getTerminatedLeases($tsStart, $tsStop) {
		if (!$this->isPermitted('read'))
			return false;
		$storage = SyndNodeLib::getDefaultStorage('lease');
		$database = $storage->getDatabase();
		$sql = "
			SELECT l.node_id FROM synd_inv_lease l
			WHERE 
				l.ts_terminated BETWEEN ".$database->quote($tsStart)." AND ".$database->quote($tsStop);
		return SyndLib::filter($storage->getInstances($database->getCol($sql)), 'isPermitted', 'read');
	}

	/**
	 * Returns all leases given a user id
	 * @param	string	Customer id, for example 'user.123'
	 * @return	array
	 */
	function _public_getCustomerLeases($id) {
		if (!$this->isPermitted('read') || empty($id))
			return false;
		if (null === ($user = SyndLib::getInstance($id)) && (
			null === ($module = Module::getInstance('user')) ||
			null === ($user = $module->resolveLogin($id))))
			return array();
		
		$storage = SyndNodeLib::getDefaultStorage('lease');
		$database = $storage->getDatabase();
		$sql = "
			SELECT l.node_id FROM synd_inv_lease l
			WHERE l.client_node_id = ".$database->quote($user->nodeId);
		return SyndLib::filter($storage->getInstances($database->getCol($sql)), 'isPermitted', 'read');
	}

	/**
	 * Autosuggest clients callback
	 * @see		synd_module_user::findSuggestedUsers()
	 */
	function findSuggestedUsers($query) {
		$module = Module::getInstance('user');
		$suggestions = $module->findSuggestedUsers($query);
		$suggestions["\"$query\""] = SyndLib::translate('Text <em>"%s"</em> only', $query);
		return $suggestions;
	}

	/**
	 * Autosuggest clients callback
	 * @see		synd_module_user::findSuggestedUsers()
	 */
	function _public_findSuggestedUsers($query) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		ignore_user_abort(false);
		return $this->findSuggestedUsers($query);
	}

	/**
	 * Autosuggest costcenters callback
	 * @see		synd_module_user::findSuggestedUsers()
	 */
	function _public_findSuggestedCostcenters($query) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		ignore_user_abort(false);
		return $this->_find('search_costcenter', $query);
	}
	
	/**
	 * Autosuggest projects callback
	 * @see		synd_module_user::findSuggestedUsers()
	 */
	function _public_findSuggestedProjects($query) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		ignore_user_abort(false);
		return $this->_find('search_project', $query);
	}

	/**
	 * @access	private
	 */
	function _find($hook, $query) {
		$cid = "lease.$hook.".md5($query);
		$result = array();
		
		if (null == ($result = SyndLib::runHook('cache_get', $cid))) {
			$collections = (array)SyndLib::runHook($hook, $query);
			$collection = SyndType::factory('composite_collection', $collections);
			$matches = $collection->getContents(0, 10);

			foreach (array_keys($matches) as $key) {
				$name = $matches[$key]->toString();
				if (null != $matches[$key]->getContact())
					$name .= ' <span class="Info">('.$matches[$key]->getContact().')</span>';
				$result[$matches[$key]->getLogin()] = $name;
			}
			
			SyndLib::runHook('cache_set', $cid, $result);
		}
		
		return $result;
	}
}
