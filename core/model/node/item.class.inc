<?php
require_once 'core/model/node/instance.class.inc';

class synd_node_item extends synd_node_instance {
	/**
	 * @access	private
	 */
	var $_client = -1;
	
	/**
	 * Buffered id of client relation
	 * @access	private
	 */
	var $_relation = null;

	function __construct($storage, $database, $data) {
		if (null != $data['DATA_FILES'])
			$data['DATA_FILES'] = unserialize($data['DATA_FILES']);
		parent::__construct($storage, $database, $data);
		$GLOBALS['_synd_item_deferred'][] = $this->nodeId;
	}

	function __wakeup() {
		parent::__wakeup();
		if (-1 != $this->_client)
			SyndNodeLib::preload($this->_client);
		else if (null != $this->_client)
			$GLOBALS['_synd_item_deferred'][] = $this->nodeId;
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		$query = parent::getEntityQuery($storage);
		$query->join('synd_inv_item');
		return $query;
	}

	function hasPermission($user, $perm, $arg = null) {
		$client = $this->getCustomer();
		$class = $this->getClass();

		if ('write' == $perm && $this->isNew())
			return true;
		if ('append' == $perm && ($arg instanceof synd_node_comment))
			return $this->isPermitted('write');
		
		if ('merge' == $perm && in_array($arg, array(
			'OWNER_NODE_ID','TS_DELIVERY','INFO_WARRANTY','INFO_MAKE','INFO_MODEL','INFO_SERIAL_MAKER',
			'INFO_SERIAL_INTERNAL','INFO_LOCATION','INFO_COST','INFO_RUNNING_COST')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}

	function toString() {
		$name = null;
		if (null != $this->data['INFO_MAKE'])
			$name .= ' '.$this->data['INFO_MAKE'];
		if (null != $this->data['INFO_MODEL'])
			$name .= ' '.$this->data['INFO_MODEL'];
		if (null == trim($name))
			return SyndLib::translate('No title');
		return trim($name);
	}
	
	function getHandler() {
		return 'inventory';
	}
	
	function appendChild($child) {
		if (($child instanceof synd_type_file)) {
			$this->data['DATA_FILES'][$child->nodeId()] = $child;
			$child->setParent($this);
			return $child;
		}
		return $child = parent::appendChild($child);
	}

	function getCompositeData() {
		$data = parent::getCompositeData();
		
		$installation = $this->getInstallation();
		if (!$installation->isNull())
			$data['installation'] = $installation->getInstallationNumber();

		$class = $this->getClass();
		if (!$class->isNull())
			$data['values'] = $this->getValues();

		$costcenter = $this->getCostcenter();
		$data['costcenter'] = $costcenter->getLogin();
		$project = $this->getProject();
		$data['project'] = $project->getLogin();

		return $data;
	}

	function validate($data) {
		require_once 'core/lib/SyndHTML.class.inc';
		require_once 'core/lib/SyndDate.class.inc';
		$errors = array();

		if (!empty($data['PARENT_NODE_ID'])) {
			$lastParent = $this->getParent();
			$parent = $this->_storage->getInstance($data['PARENT_NODE_ID']);
			if (null == $parent || !$parent->isPermitted('write') && $parent->nodeId != $lastParent->nodeId)
				$errors['PARENT_NODE_ID'] = SyndLib::translate('Invalid folder selected.');
		}

		if (isset($data['TS_DELIVERY']) && null != $data['TS_DELIVERY']) {
			if (!SyndDate::checkdate($data['TS_DELIVERY'])) 
				$errors['TS_DELIVERY'] = SyndLib::translate("Invalid date given ('%s')", tpl_chop($data['TS_DELIVERY'],12));
			unset($data['TS_DELIVERY']);
		}
		
		if (isset($data['INFO_SERIAL_INTERNAL']) && '' != $data['INFO_SERIAL_INTERNAL']) {
			$sql = "
				SELECT i.node_id FROM synd_inv_item i 
				WHERE 
					i.node_id != ".$this->_db->quote($this->nodeId)." AND
					i.info_serial_internal = ".$this->_db->quote($data['INFO_SERIAL_INTERNAL']);
			if (count($items = $this->_storage->getInstances($this->_db->getCol($sql,0)))) {
				$message = '';
				foreach ($items as $item) {
					$message .= ', <a href="'.tpl_link('inventory','view',$item->nodeId).'">';
					$message .= $item->data['INFO_SERIAL_INTERNAL'];
					$message .= '</a>';
				}
				$errors['INFO_SERIAL_INTERNAL'] = SyndLib::translate('Serial number already exists (%s)', trim($message,', '));
			}
		}

		if (isset($data['INFO_SERIAL_MAKER']) && '' != $data['INFO_SERIAL_MAKER']) {
			$sql = "
				SELECT i.node_id FROM synd_inv_item i 
				WHERE 
					i.node_id != ".$this->_db->quote($this->nodeId)." AND
					i.info_serial_maker = ".$this->_db->quote($data['INFO_SERIAL_MAKER']);
			if (count($items = $this->_storage->getInstances($this->_db->getCol($sql,0)))) {
				$message = '';
				foreach ($items as $item) {
					$message .= ', <a href="'.tpl_link('inventory','view',$item->nodeId).'">';
					$message .= $item->data['INFO_SERIAL_MAKER'];
					$message .= '</a>';
				}
				$errors['INFO_SERIAL_MAKER'] = SyndLib::translate('Serial number already exists (%s)', trim($message,', '));
			}
		}

		SyndHTML::validateFile($data, $errors, 'file');

		return array_merge(parent::validate($data), $errors);
	}

	function merge($data) {
		if (isset($data['TS_DELIVERY']) && null != $data['TS_DELIVERY'])
			$data['TS_DELIVERY'] = strtotime($data['TS_DELIVERY']);
		
		// Search for client
		if (isset($data['client'], $data['prevClient'])) {
			$module = Module::getInstance('user');
			$client = $this->getCustomer();

			if ('' == $data['client'])
				$this->setCustomer(SyndNodeLib::getInstance('null.null'));
			else if (strlen($data['client']) > 2 && '"' == $data['client']{0} && '"' == $data['client'][strlen($data['client'])-1]) 
				$this->setCustomer(SyndNodeLib::getInstance('user_case.'.trim($data['client'],'"')));
			else if ($client->isNull() || $data['client'] != $client->getLogin() && $data['client'] != $client->toString()) {
				if (null !== ($newClient = $module->resolveEmail($data['client'])) ||
					null !== ($newClient = $module->resolveLogin($data['client'])))
					$this->setCustomer($newClient);
			}
		}
		
		// Resolve costcenter and project
		if (isset($data['costcenter'])) {
			if (null !== ($costcenter = SyndLib::runHook('find_costcenter', $data['costcenter'])))
				$data['COSTCENTER_NODE_ID'] = $costcenter->id();
			else if ('' != trim($data['costcenter']))
				$data['COSTCENTER_NODE_ID'] = 'node.user_case.'.$data['costcenter'];
			else
				$data['COSTCENTER_NODE_ID'] = null;
		}
		
		if (isset($data['project'])) {
			if (null != ($project = SyndLib::runHook('find_project', $data['project'])))
				$data['PROJECT_NODE_ID'] = $project->id();
			else if ('' != trim($data['project']))
				$data['PROJECT_NODE_ID'] = 'node.user_case.'.$data['project'];
			else
				$data['PROJECT_NODE_ID'] = null;
		}
		
		if (isset($data['installation'])) {
			if (null == $data['installation'])
				$data['INSTALLATION_NODE_ID'] = null;
			else {
				$installation = $this->_findInstallation($data['installation']);
				$data['INSTALLATION_NODE_ID'] = $installation->nodeId;
			}
			unset($data['installation']);
		}

		// Attach new file
		if (!empty($data['file']['tmp_name']) && null !== ($file = SyndType::factory('file', $data['file']['tmp_name'], $data['file']['name'])))
			$this->appendChild($file);
		
		parent::merge($data);
	}

	function acceptIndexer(IndexBuilder $indexer) {
		parent::acceptIndexer($indexer);
		
		$indexer->visitFragment($this->data['INFO_MAKE'].' '.$this->data['INFO_MODEL'], 'title');
		$indexer->visitFragment($this->data['INFO_SERIAL_INTERNAL'], 'id');
		$indexer->visitFragment($this->data['INFO_SERIAL_MAKER'], 'id');
		$indexer->visitFragment($this->data['INFO_LOCATION']);
		
		$this->getCustomer()->acceptIndexer($indexer);
		$this->getInstallation()->acceptIndexer($indexer);
		
		foreach ($this->getValues() as $value)
			$indexer->visitFragment($value);
	}
	
	function getIndexSection() {
		$class = $this->getClass();
		if (!$class->isNull())
			return parent::getIndexSection().'.'.$class->objectId();
		return parent::getIndexSection();
	}
	
	/**
	 * Saves any buffered client relations
	 */
	function save() {
		parent::save();
		if (null !== $this->_relation && null !== ($relation = $this->_storage->getInstance($this->_relation)))
			$relation->save();
	}
	
	function getSerial() {
		if (null != $this->data['INFO_SERIAL_MAKER'])
			return $this->data['INFO_SERIAL_MAKER'];
		return $this->data['INFO_SERIAL_INTERNAL'];
	}
	
	function getComments() {
		$sql = "
			SELECT c.node_id FROM synd_comment c
			WHERE c.parent_node_id = ".$this->_db->quote($this->nodeId);
		return $this->_storage->getInstances($this->_db->getCol($sql));
	}
	
	function getRepairs() {
		$sql = "
			SELECT r.child_node_id, r.ts_create FROM synd_relation r
			WHERE r.parent_node_id = ".$this->_db->quote($this->nodeId)."
			ORDER BY r.ts_create DESC";
		return $this->_storage->getInstances($this->_db->getCol($sql));
	}
	
	function getFolderOptions() {
		$sql = "SELECT f.node_id FROM synd_inv_folder f";
		return SyndLib::filter($this->_storage->getInstances($this->_db->getCol($sql)),'isPermitted','append',$this);
	}
	
	function setParent($parent) {
		$prevParent = $this->getParent();
		if (($prevParent instanceof synd_node_folder) && ($parent instanceof synd_node_folder) && $prevParent->nodeId != $parent->nodeId) {
			$comment = $this->appendChild(SyndNodeLib::factory('comment'));
			$comment->setContent(SyndLib::translate("Moved item from folder '%s' to '%s'", 
				$prevParent->toString(), $parent->toString()));
			$comment->save();
		}
		parent::setParent($parent);
	}
	
	function setFolder($folder) {
		$this->setParent($folder);
	}

	function getFolder() {
		if (null === ($folder = $this->_storage->getInstance($this->data['PARENT_NODE_ID'])))
			$folder = SyndNodeLib::getInstance('user_null.null');
		else {
			while (!($folder instanceof synd_node_folder) && !$folder->isNull())
				$folder = $folder->getParent();
		}
		return $folder;
	}
	
	/**
	 * Returns the user currently leasing this item
	 * @return	User
	 */
	function getCustomer() {
		if (null !== $this->_relation && null != ($relation = $this->_storage->getInstance($this->_relation)))
			return $relation->getCustomer();
		
		if (-1 == $this->_client) {
			$GLOBALS['_synd_item_deferred'][] = $this->nodeId;
			$deferred = $this->_db->quote($GLOBALS['_synd_item_deferred']);

			$sql = "
				SELECT u.child_node_id, u.parent_node_id 
				FROM synd_inv_used u
				WHERE 
					u.child_node_id IN (".implode(', ', $deferred).") AND
					u.ts_expire IS NULL";
			$clients = $this->_db->getAssoc($sql);
			SyndNodeLib::getInstances($clients);
			
			foreach ($GLOBALS['_synd_item_deferred'] as $id) {
				if (null !== ($item = SyndNodeLib::getInstance($id))) {
					$item->_client = isset($clients[$id]) ? $clients[$id] : null;
					$item->_node_onchange_internal();
				}
			}

			unset($GLOBALS['_synd_item_deferred']);
		}
		
		if (-1 == $this->_client || null == ($client = SyndNodeLib::getInstance($this->_client)))
			$client = SyndNodeLib::getInstance('user_null.null');
		return $client;
	}
	
	function setCustomer($client) {
		return $this->_setCustomer($client);
	}
	
	/**
	 * Returns the current leasing relation
	 * @return	synd_node_used
	 */
	function getLeasing() {
		if (null === $this->_relation) {
			$sql = "
				SELECT u.node_id FROM synd_inv_used u
				WHERE 
					u.child_node_id = ".$this->_db->quote($this->nodeId)." AND
					u.ts_expire IS NULL";
			$this->_relation = (string)$this->_db->getOne($sql);
			$this->_node_onchange_internal();
		}
		if (null === ($leasing = $this->_storage->getInstance($this->_relation)))
			$leasing = SyndNodeLib::getInstance('null.null');
		return $leasing;
	}

	/**
	 * @access	protected
	 * @return	synd_node_used
	 */
	function _setCustomer($client) {
		$lastClient = $this->getCustomer();
		$relation = null;
		
		if ($lastClient->nodeId != $client->nodeId) {
			$relations = SyndLib::filter($this->getClientRelations(), 'isActive');
			SyndLib::invoke($relations, 'setInactive');
			
			if ($client->isNull()) 
				$this->_relation = '';
			else {
				$relation = $this->_storage->factory('used');
				$this->_relation = $relation->nodeId;
				
				$relation->setItem($this);
				$relation->setCustomer($client);
			}
		}

		$this->_client = -1;
		$this->_node_onchange_internal();

		return $relation;
	}

	/**
	 * @access	protected
	 */
	function getClientRelations() {
		$sql = "
			SELECT u.node_id FROM synd_inv_used u
			WHERE u.child_node_id = ".$this->_db->quote($this->nodeId)."
			ORDER BY u.ts_create DESC";
		return $this->_storage->getInstances($this->_db->getCol($sql));
	}
	
	/**
	 */
	static function getClientItems($client) {
		$storage = SyndNodeLib::getDefaultStorage('item');
		$database = $storage->getDatabase();
		$sql = "
			SELECT u.child_node_id FROM synd_inv_used u
			WHERE 
				u.parent_node_id = ".$database->quote($client->nodeId)." AND
				u.ts_expire IS NULL";
		return $storage->getInstances($database->getCol($sql));
	}

	function isLeased() {
		$lease = $this->getLease();
		return !$lease->isNull();
	}
	
	function getLease() {
		$client = $this->getCustomer();
		if (!($client instanceof synd_node_lease))
			$client = SyndNodeLib::getInstance('null.null');
		return $client;
	}

	function getOwner() {
		if (null === ($owner = SyndNodeLib::getInstance($this->data['OWNER_NODE_ID'])))
			$owner = SyndNodeLib::getInstance('user_null.null');
		return $owner;
	}
	
	function getOwnerOptions() {
		global $synd_user;
		return $synd_user->getPrimaryGroupOptions();
	}
	
	/**
	 * Returns the costcenter that owns this item
	 * @return	User
	 */
	function getCostcenter() {
		if (null == ($costcenter = SyndLib::getInstance($this->data['COSTCENTER_NODE_ID'])))
			$costcenter = SyndNodeLib::getInstance('user_null.null');
		return $costcenter;
	}

	/**
	 * Returns the project that owns this item
	 * @return	User
	 */
	function getProject() {
		if (null === ($project = SyndLib::getInstance($this->data['PROJECT_NODE_ID'])))
			$project = SyndNodeLib::getInstance('user_null.null');
		return $project;
	}

	function getTitle() {
		$name = null;
		if (null != $this->data['INFO_MAKE'])
			$name .= ' '.$this->data['INFO_MAKE'];
		if (null != $this->data['INFO_MODEL'])
			$name .= ' '.$this->data['INFO_MODEL'];
		if (null != $name)
			return trim($name);
		return null;
	}

	function getPurchaseValue() {
		return $this->data['INFO_COST'];
	}
	
	function getRunningCost() {
		return $this->data['INFO_RUNNING_COST'];
	}
	
	/**
	 * Returns the deprication cost, 0 after 4 years of use
	 * @param	integer	Start of period, defaults to start of current year
	 * @param	integer	End of period, default to remaining
	 * @return	integer
	 */
	function getDepreciationCost($tsStart = null, $tsStop = null) {
		require_once 'core/lib/SyndDate.class.inc';
		if (null == $this->data['TS_DELIVERY'])
			return null;

		// 3 years default deprecation
		$end = strtotime('+3 years', $this->data['TS_DELIVERY']);

		// Default values
		if (null == $tsStart)
			$tsStart = SyndDate::startOfYear(time());
		if (null == $tsStop)
			$tsStop = $end;

		if ($tsStart >= $end || $tsStop <= $this->data['TS_DELIVERY'])
			return 0;
		
		$daysPeriod = abs(max($tsStart, $this->data['TS_DELIVERY']) - min($tsStop, $end)) / 60 / 60 / 24;
		$daysDeprecation = ($end - $this->data['TS_DELIVERY']) / 60 / 60 / 24;
		
		return round($this->getPurchaseValue() / $daysDeprecation * $daysPeriod);
	}
	
	function getHistory() {
		$sql = "
			SELECT c.node_id, c.ts_create FROM synd_comment c
			WHERE c.parent_node_id = ".$this->_db->quote($this->nodeId)."
			ORDER BY c.ts_create DESC";
		return $this->_storage->getInstances($this->_db->getCol($sql));
	}
	
	function _view_addHistory(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!empty($request['INFO_BODY'])) {
			$event = SyndNodeLib::factory('comment');
			$event->data['INFO_BODY'] = $request['INFO_BODY'];
			if (null !== $this->appendChild($event))
				$event->save();
		}
	}
	
	function getPeripherals() {
		global $synd_maindb;
		$sql = "
			SELECT p.NODE_ID, p.INFO_COST, p.INFO_DESC
			FROM synd_inv_peripheral p
			WHERE p.parent_node_id = '$this->nodeId'
			ORDER BY p.node_id";
		return $synd_maindb->getAll($sql);
	}
	
	function _view_addPeripheral(Request $request, Template $page) {
		global $synd_maindb;
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!$request['INFO_DESC'])
			return;
		
		$id = $this->_db->nextId('synd_inv_peripheral');
		$cost = empty($request['INFO_COST']) ? 0 : $request['INFO_COST'];

		$sql = "
			INSERT INTO synd_inv_peripheral
			(node_id, parent_node_id, info_cost, info_desc)
			VALUES
			('peripheral.$id', 
			 ".$this->_db->quote($this->nodeId).", 
			 ".$this->_db->quote($cost).", 
			 ".$this->_db->quote($request['INFO_DESC']).")";
		$this->_db->query($sql);
	}
	
	function _view_delPeripheral(Request $request, Template $page) {
		global $synd_maindb;
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		
		$sql = "
			DELETE FROM synd_inv_peripheral
			WHERE 
				node_id = ".$this->_db->quote($request['id'])." AND 
				parent_node_id = ".$this->_db->quote($this->nodeId);
		$this->_db->query($sql);
	}

	function _view_unlink(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!isset($request[2], $this->data['DATA_FILES'][$request[2]])) 
			throw new NotFoundException();

		if (isset($request['confirm'])) {
			if (!$this->data['DATA_FILES'][$request[2]]->delete())
				throw new RuntimeException();
			unset($this->data['DATA_FILES'][$request[2]]);
			$this->_save();
		}
		else {
			$file = $this->data['DATA_FILES'][$request[2]];
			return array('content' => $page->fetchnode($this, 'confirm_delete_file.tpl', array('file' => $file)));
		}
	}
	
	function _public_getSerial() {
		if (!$this->isPermitted('read'))
			return false;
		return $this->data['INFO_SERIAL_MAKER'];
	}
	
	function getModel() {
		return trim($this->data['INFO_MAKE'].' '.$this->data['INFO_MODEL']);
	}
	
	function _public_getModel() {
		if (!$this->isPermitted('read'))
			return false;
		return $this->data['INFO_MODEL'];
	}

	function getInstallation() {
		if (null != ($installation = SyndNodeLib::getInstance($this->data['INSTALLATION_NODE_ID'])))
			return $installation;
		return SyndNodeLib::getInstance('null.null');
	}
	
	function _findInstallation($number) {
		$sql = "
			SELECT i.node_id FROM synd_inv_installation i
			WHERE LOWER(i.info_number) = ".$this->_db->quote(strtolower($number));
		if (null == ($installation = SyndNodeLib::getInstance($this->_db->getOne($sql)))) {
			$installation = SyndNodeLib::factory('installation');
			$installation->setInstallationNumber($number);
			$installation->save();
		}
		return $installation;
	}
	
	static function findSerialNumbers($storage, $numbers) {
		$result = array();
		
		$query = self::getEntityQuery($storage);
		$query->in('UPPER(info_serial_maker)', array_map('strtoupper', $numbers));
		
		foreach ($storage->getInstances($query->getCol()) as $node)
			$result[$node->nodeId] = $node;
		
		$query = self::getEntityQuery($storage);
		$query->in('UPPER(info_serial_internal)', array_map('strtoupper', $numbers));

		foreach ($storage->getInstances($query->getCol()) as $node)
			$result[$node->nodeId] = $node;

		return $result;
	}

	/**
	 * Returns the files attached to this issue
	 * @see		synd_type_file
	 * @return	array	Array of synd_type_file objects
	 */
	function getFiles() {
		if (null != $this->data['DATA_FILES'])
			return $this->data['DATA_FILES'];
		return array();
	}

	/**
	 * Autosuggest clients callback
	 * @see		synd_module_user::findSuggestedUsers()
	 */
	function _public_findSuggestedUsers($query) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		ignore_user_abort(false);
		$module = Module::getInstance('inventory');
		return $module->findSuggestedUsers($query);
	}
	
	/**
	 * Autosuggest costcenters callback
	 * @see		synd_module_user::findSuggestedUsers()
	 */
	function _public_findSuggestedCostcenters($query) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		ignore_user_abort(false);
		$module = Module::getInstance('inventory');
		return $module->_find('search_costcenter', $query);
	}
	
	/**
	 * Autosuggest projects callback
	 * @see		synd_module_user::findSuggestedUsers()
	 */
	function _public_findSuggestedProjects($query) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		ignore_user_abort(false);
		$module = Module::getInstance('inventory');
		return $module->_find('search_project', $query);
	}

	/**
	 * Clears the cached client id
	 */
	function _node_onchange_child() {
		$this->_client = -1;
		parent::_node_onchange_child();
	}
}
