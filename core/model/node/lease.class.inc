<?php
require_once 'core/model/DatabaseEntity.class.inc';

/**
 * An inventory lease between a client and one or several items
 * 
 * The lease manages the relations between a client and multiple items
 * in the inventory. Both current and past items for this lease are
 * stored for history keeping purposes.
 */
class synd_node_lease extends AbstractDatabaseEntity {
	/**
	 * Buffered list of leasings
	 * @access	private
	 */
	var $_leasings = null;

	/**
	 * Buffered list of SLAs
	 * @access	private
	 */
	var $_descriptions = null;
	
	function __construct($storage, $database, $data) {
		parent::__construct($storage, $database, $data);
		SyndNodeLib::preload($this->data['CLIENT_NODE_ID']);
	}

	function __factory() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_lease');
		$this->data['TS_CREATE'] = $this->data['TS_UPDATE'] = time();
		$this->data['CREATE_NODE_ID'] = $this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;

		if (null !== ($inventory = Module::getInstance('inventory')) && 
			null !== ($folder = $inventory->getLeaseFolder()))
			$this->data['PARENT_NODE_ID'] = $folder->nodeId;
		else
			$this->data['PARENT_NODE_ID'] = 'null.null';

		parent::__factory();
	}
	
	function __clone() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_lease');
		$this->data['TS_CREATE'] = time();
		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_inv_lease');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('write' == $perm && $this->isNew())
			return true;
		
		if ('append' == $perm && (null == $arg || ($arg instanceof synd_node_item)) && !$this->isTerminated() && $this->hasPermission($user, 'write'))
			return true;
		if ('remove' == $perm) {
			$client = $this->getCustomer();
			return $client->isNull();
		}

		if ('merge' == $perm && in_array($arg, array(
			'PARENT_NODE_ID','CLIENT_NODE_ID','COSTCENTER_NODE_ID','RECEIPT_NODE_ID',
			'TS_CREATE','TS_EXPIRE','TS_TERMINATED','INFO_BODY')))
			return $this->hasPermission($user, 'write');

		return parent::hasPermission($user, $perm, $arg);
	}

	function delete() {
		$sql = "
			DELETE FROM synd_relation
			WHERE 
				parent_node_id = ".$this->_db->quote($this->nodeId)." OR
				child_node_id = ".$this->_db->quote($this->nodeId);
		$this->_db->query($sql);
		parent::delete();
	}

	function getHandler() {
		return 'inventory';
	}
	
	/**
	 * Adjusts leasing buffers and traps append of SLAs to create relations
	 */
	function appendChild($child) {
		if (($child instanceof synd_node_used)) {
			if (null === $this->_leasings)
				$this->getLeasings();
			if (!in_array($child->nodeId, $this->_leasings)) {
				$this->_leasings[] = $child->nodeId;
				$this->_node_onchange_internal();
			}
		}
		else if (($child instanceof synd_node_sld)) {
			if (null === $this->_descriptions)
				$this->getServiceLevelDescriptions();
			if (!in_array($child->nodeId, $this->_descriptions)) {
				$this->_descriptions[] = $child->nodeId;
				$this->_node_onchange_internal();
			}
			return $child;
		}
		return parent::appendChild($child);
	}
	
	function removeChild($child) {
		if (($child instanceof synd_node_used)) {
			if (null === $this->_leasings)
				$this->getLeasings();
			if (false !== ($key = array_search($child->nodeId, $this->_leasings)))
				unset($this->_leasings[$key]);
		}
		else if (($child instanceof synd_node_sld)) {
			if (null === $this->_descriptions)
				$this->getServiceLevelDescriptions();
			if (false !== ($key = array_search($child->nodeId, $this->_descriptions)))
				unset($this->_descriptions[$key]);
			return $child;
		}
		return parent::appendChild($child);
	}
	
	function getFolder() {
		return $this->getParent();
	}

	function setFolder($folder) {
		$this->setParent($folder);
	}

	function getFolderOptions() {
		$sql = "SELECT f.node_id FROM synd_inv_folder f";
		return SyndLib::filter($this->_storage->getInstances($this->_db->getCol($sql)),'isPermitted','append',$this);
	}
	
	function getReceiptTemplate() {
		if (null != ($receipt = SyndNodeLib::getInstance($this->data['RECEIPT_NODE_ID'])))
			return $receipt;
		return SyndNodeLib::getInstance('null.null');
	}
	
	function getReceiptOptions() {
		$inventory = Module::getInstance('inventory');
		return $inventory->getTemplates(get_class($this));
	}
	
	/**
	 * Returns the entity responsible for payments for this lease
	 * @return	User
	 */
	function getCostcenter() {
		if (null === ($costcenter = SyndLib::getInstance($this->data['COSTCENTER_NODE_ID'])))
			$costcenter = $this->getCustomer();
		return $costcenter;
	}
	
	/**
	 * Returns the list of available cost centers
	 * @return	array
	 */
	function getCostcenterOptions() {
		$sql = "SELECT c.node_id FROM synd_costcenter c";
		$options = SyndLib::filter($this->_storage->getInstances($this->_db->getCol($sql)),'isPermitted','write');
		if (null != ($costcenter = $this->_storage->getInstance($this->data['COSTCENTER_NODE_ID'])) && !isset($options[$costcenter->nodeId]))
			$options[$costcenter->nodeId] = $costcenter;
		return $options;
	}

	/**
	 * Returns the project responsible for payments for this lease
	 * @return	User
	 */
	function getProject() {
		if (null === ($project = SyndLib::getInstance($this->data['PROJECT_NODE_ID'])))
			$project = SyndNodeLib::getInstance('user_null.null');
		return $project;
	}
	
	/**
	 * Returns selected service level agreements
	 * @return	array
	 */
	function getServiceLevelDescriptions() {
		if (null === $this->_descriptions) {
			$sql = "
				SELECT ls.sld_node_id FROM synd_inv_lease_sld ls
				WHERE 
					ls.lease_node_id = ".$this->_db->quote($this->nodeId)." AND
					ls.ts_terminated IS NULL";
			$this->_descriptions = (array)$this->_db->getCol($sql);
			$this->_node_onchange_internal();
		}
		return $this->_storage->getInstances($this->_descriptions);
	}
	
	/**
	 * Returns the list of available Service level agreements
	 * @return	array
	 */
	function getServiceLevelAgreementOptions() {
		$sql = "SELECT s.node_id FROM synd_inv_sld s";
		return SyndLib::filter($this->_storage->getInstances($this->_db->getCol($sql)),'isPermitted','write');
	}
	
	function toString() {
		$client = $this->getCustomer();
		if ($this->isNew() && $client->isNull())
			return SyndLib::translate('New lease');
		return null != $client->toString() ? $client->toString() : SyndLib::translate('No title');
	}
	
	/**
	 * @deprecated
	 */
	function getLogin() {
		$client = $this->getCustomer();
		return $client->getLogin();
	}
	
	/**
	 * @deprecated
	 */
	function getPhone() {
		$client = $this->getCustomer();
		return $client->getPhone();
	}
	
	/**
	 * @deprecated
	 */
	function getEmail() {
		$client = $this->getCustomer();
		return $client->getEmail();
	}

	/**
	 * @deprecated
	 */
	function getContact() {
		$client = $this->getCustomer();
		return $client->getContact();
	}

	/**
	 * @deprecated
	 */
	function getSocialSecurityNumber() {
		$client = $this->getCustomer();
		return $client->getSocialSecurityNumber();
	}

	/**
	 * @deprecated
	 */
	function getAddress() {
		$costcenter = $this->getCostcenter();
		return $costcenter->getAddress();
	}
	
	function getDescription() {
		return $this->data['INFO_BODY'];
	}
	
	function getOrganization() {
		$client = $this->getCustomer();
		return $client->getOrganization();
	}

	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function setParent($node) {
		$this->data['PARENT_NODE_ID'] = $node->nodeId;
	}
	
	function setCustomer($client) {
		$this->data['CLIENT_NODE_ID'] = $client->nodeId;
	}
	
	function getCustomer() {
		if (null === ($client = SyndNodeLib::getInstance($this->data['CLIENT_NODE_ID'])))
			$client = SyndNodeLib::getInstance('user_null.null');
		return $client;
	}

	/**
	 * Searches for potential new clients
	 * @param	string	Query to search for
	 * @return	Collection
	 */
	function findClientInstances($string) {
		$module = Module::getInstance('user');
		return $module->findUsers($string);
	}

	function acceptIndexer(IndexBuilder $indexer) {
		parent::acceptIndexer($indexer);

		$indexer->visitFragment($this->data['INFO_BODY']);
		$this->getCustomer()->acceptIndexer($indexer);
	}

	function getRelations() {
		$sql = "
			SELECT u.node_id, u.ts_create FROM synd_inv_used u
			WHERE u.parent_node_id = ".$this->_db->quote($this->nodeId)."
			ORDER BY u.ts_create DESC";
		return $this->_storage->getInstances($this->_db->getCol($sql));
	}

	/**
	 * Returns the current items for this lease
	 * @return	array
	 */
	function getItems() {
		$items = array();
		foreach (array_keys($relations = $this->getLeasings()) as $key)
			$items[] = $relations[$key]->getItem();
		return $items;
	}

	/**
	 * Retrives returned items
	 * @return	array
	 */
	function getReturnedItems() {
		$items = array();
		foreach (array_keys($relations = $this->getInactiveLeasings()) as $key)
			$items[] = $relations[$key]->getItem();
		return $items;
	}
	
	function getLeasings() {
		if (null === $this->_leasings) {
			$sql = "
				SELECT u.node_id FROM synd_inv_used u
				WHERE 
					u.parent_node_id = ".$this->_db->quote($this->nodeId)." AND
					u.ts_expire IS NULL";
			$this->_leasings = (array)$this->_db->getCol($sql);
			$this->_node_onchange_internal();
		}
		return $this->_storage->getInstances($this->_leasings);
	}

	function getInactiveLeasings() {
		$sql = "
			SELECT u.node_id FROM synd_inv_used u
			WHERE 
				u.parent_node_id = ".$this->_db->quote($this->nodeId)." AND
				u.ts_expire IS NOT NULL";
		return $this->_storage->getInstances($this->_db->getCol($sql));
	}

	function getInvoices() {
		$sql = "
			SELECT i.node_id, i.ts_create FROM synd_issue i, synd_issue_invoice iv
			WHERE 
				i.node_id = iv.node_id AND
				iv.lease_node_id = ".$this->_db->quote($this->nodeId)."
			ORDER BY i.ts_create DESC";
		return $this->_storage->getInstances($this->_db->getCol($sql, 0, 0, 25));
	}
	
	function isTerminated() {
		return null != $this->data['TS_TERMINATED'];
	}
	
	function getTerminatedTime() {
		return $this->data['TS_TERMINATED'];
	}
	
	/**
	 * Terminates this lease
	 * 
	 * Lease can not contain any active leases. Will return
	 * false if so, true otherwise.
	 *
	 * @return	boolean
	 */
	function terminate() {
		if (count($this->getLeasings()))
			return false;
		$this->data['TS_TERMINATED'] = time();
		$this->_save();
		return true;
	}
	
	function getCompositeData() {
		$data = parent::getCompositeData();
		
		$client = $this->getCustomer();
		if ($client instanceof synd_node_user_case)
			$data['customer'] = '"'.$client->getLogin().'"';
		else
			$data['customer'] = $client->getLogin();
		
		$costcenter = $this->getCostcenter();
		if ($costcenter->id() != $client->id())
			$data['costcenter'] = $costcenter->getLogin();
		
		$project = $this->getProject();
		$data['project'] = $project->getLogin();

		return $data;
	}
	
	function validate($data) {
		require_once 'core/lib/SyndDate.class.inc';
		$errors = array();

		if (!empty($data['sla']) && (null === ($sla = $this->_storage->getInstance($data['sla'])) || !$sla->isPermitted('read')))
			$errors['sla'] = SyndLib::translate('Invalid SLA selected');
		if (!empty($data['created']) && !SyndDate::checkdate($data['created'])) 
			$errors['created'] = SyndLib::translate("Invalid date given ('%s')", tpl_chop($data['created'],12));
		if (!empty($data['expires']) && !SyndDate::checkdate($data['created'])) 
			$errors['expires'] = SyndLib::translate("Invalid date given ('%s')", tpl_chop($data['expires'],12));
		if (!empty($data['terminated']) && !SyndDate::checkdate($data['created'])) 
			$errors['terminated'] = SyndLib::translate("Invalid date given ('%s')", tpl_chop($data['terminated'],12));
		
		$module = Module::getInstance('user');
		if (isset($data['customer']) && $data['customer'] != null) {
                        if (strlen($data['customer']) > 2 && '"' == $data['customer']{0} && '"' == $data['customer']{strlen($data['customer'])-1})
                                $data['CLIENT_NODE_ID'] = 'user_case.'.trim($data['customer'],'"');
			else if (null !== ($customer = $module->resolveLogin($data['customer'])) ||
				 null !== ($customer = $module->resolveEmail($data['customer'])))
				$data['CLIENT_NODE_ID'] = $customer->nodeId;
			else
				$errors['CLIENT_NODE_ID'] = SyndLib::translate("Failed to resolve username '%s'", $data['customer']);
		}

		return array_merge(parent::validate($data), $errors);
	}

	function merge($data) {
		if (!empty($data['created']))
			$data['TS_CREATE'] = strtotime($data['created']);
		if (isset($data['expires']))
			$data['TS_EXPIRE'] = null != $data['expires'] ? strtotime($data['expires']) : null;
		if (isset($data['terminated']))
			$data['TS_TERMINATED'] = null != $data['terminated'] ? strtotime($data['terminated']) : null;

		
                // Search for and assign client
		$module = Module::getInstance('user');
                if (isset($data['customer']) && $data['customer'] != null) {
                        if (strlen($data['customer']) > 2 && '"' == $data['customer']{0} && '"' == $data['customer']{strlen($data['customer'])-1})
                                $data['CLIENT_NODE_ID'] = 'user_case.'.trim($data['customer'],'"');
                        else if (null !== ($customer = $module->resolveLogin($data['customer'])) ||
                                 null !== ($customer = $module->resolveEmail($data['customer'])))
                                $data['CLIENT_NODE_ID'] = $customer->nodeId;
                }
		
		if (!empty($data['sla']))
			$this->appendChild($this->_storage->getInstance($data['sla']));
		
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
			if (null !== ($project = SyndLib::runHook('find_project', $data['project'])))
				$data['PROJECT_NODE_ID'] = $project->id();
			else if ('' != trim($data['project']))
				$data['PROJECT_NODE_ID'] = 'node.user_case.'.$data['project'];
			else
				$data['PROJECT_NODE_ID'] = null;
		}
		
		parent::merge($data);
	}
	
	function toPrinter($printer) {
		$receipt = $this->getReceiptTemplate();
		if ($receipt->isNull())
			return false;
		require_once 'core/lib/SyndPrint.class.inc';
		return SyndPrint::pdfPrint($printer, $receipt->path(), $this->getReceiptFilter());
	}
	
	/**
	 * Prints the receipt
	 */
	function printReceipt() {
		$inventory = Module::getInstance('inventory');
		if (null != ($printer = $inventory->getPrinter())) 
			return $this->toPrinter($printer);
		return false;
	}
	
	function getReceiptFilter() {
		global $synd_user;
		$client = $this->getCustomer();
		$costcenter = $this->getCostcenter();
		
		$address = $client->getAddress();
		$addr = $client->toString();
		if (!empty($address['CO']))
			$addr .= "\r\n".$address['CO'];
		if (!empty($address['STREET']))
			$addr .= "\r\n".$address['STREET'];
		if (!empty($address['ZIP']) && !empty($address['CITY']))
			$addr .= "\r\n".$address['ZIP'].' '.$address['CITY'];
		if (!empty($address['COUNTRY']))
			$addr .= "\r\n".$address['COUNTRY'];
		
		$filter = array(
			'DATE'			=> tpl_strftime('%Y-%m-%d'),
			'CREATED'		=> tpl_strftime('%Y-%m-%d', (string)$this->data['TS_CREATE']),
			'EXPIRES'		=> tpl_strftime('%Y-%m-%d', (string)$this->getExpire()),
			'CUSTOMER'		=> array(
				'CAPTION'		=> null != $client->getContact() ? $client->toString().' ('.$client->getContact().')' : $client->toString(),
				'NAME'			=> $client->toString(),
				'PHONE'			=> $client->getPhone(),
				'SSN'			=> $client->getSocialSecurityNumber(),
				'ADDRESS'		=> $addr),
			'ASSIGNED'		=> array(
				'CAPTION'		=> null != $synd_user->getContact() ? $synd_user->toString().' ('.$synd_user->getContact().')' : $synd_user->toString(),
				'NAME'			=> $synd_user->toString(),
				'PHONE'			=> $synd_user->getPhone()),
			'ITEM'			=> $this->_itemReceiptFilter($this->getItems()),
			);
		
		if (($costcenter instanceof synd_node_costcenter)) {
			$liable = $costcenter->getLiableClient();
			$filter['COSTCENTER'] = array(
				'CAPTION'	=> $costcenter->toString(),
				'NUMBER'	=> $costcenter->getNumber(),
				'PROJECT'	=> $costcenter->getProjectCode(),
				'LIABLE'	=> array(
					'CAPTION'	=> null != $liable->getContact() ? $liable->toString().' ('.$liable->getContact().')' : $liable->toString(),
					'NAME'		=> $liable->toString(),
					'PHONE'		=> $liable->getPhone()));
		}
		
		return $filter;
	}

	/**
	 * @access	protected
	 */
	function _itemReceiptFilter($items) {
		$filters = array();
		foreach (array_keys($items) as $key) {
			$filters[] = array(
				'CAPTION'	=> $items[$key]->toString(),
				'SERIAL'	=> $items[$key]->getSerial(),
				);
		}
		$filters['CAPTION'] = implode(', ', SyndLib::invoke($items,'toString'));
		$filters['SERIAL'] = implode(', ', SyndLib::invoke($items,'toString'));
		return $filters;
	}

	/**
	 * @see		printReceipt()
	 */
	function _view_printReceipt(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		if (false === $this->printReceipt())
			return array('content' => $page->fetchnode($this,'error_print_receipt.tpl'));
	}

	/**
	 * Direct link to autogenerated PDF receipt
	 * 
	 * Link as http://www.example.com/synd/inventory/invoke/invoice.12345/receipt.pdf
	 */
	function _view_receipt_pdf(Request $request, Template $page) {
		require_once 'core/lib/SyndPrint.class.inc';
		$receipt = $this->getReceiptTemplate();
		$inventory = Module::getInstance('inventory');
		if (!$receipt->isNull() && null != ($printer = $inventory->getPrinter())) {
			header('Content-Type: application/pdf');
			return array('content' => SyndPrint::pdfMerge($receipt->path(), $this->getReceiptFilter()), 'page' => null);
		}
	}

	function _view_delServiceLevelAgreement(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!isset($request[2]))
			throw new InvalidArgumentException();
		$this->checkout();
		$this->removeChild($this->_storage->getInstance($request[2]));
		$this->_node_onchange_internal();
	}

	/**
	 * Creates a new termination issue and redirects to the whizard for that issue
	 */
	function _view_terminate(Request $request, Template $page) {
		global $synd_user;
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
			
		$issue = SyndNodeLib::factory('terminate');
		$issue->setLease($this);
				
		return array('redirect' => tpl_view_jump('inventory','flow',$issue->nodeId,2));
	}
	
	function _callback_breadcrumbs(&$result, Template $page) {
		$result[] = array('text' => $page->fetchnode($this,'contact.tpl'));
	}

	/**
	 * Returns the timestamp this lease was created
	 * @return	integer
	 */
	function getCreated() {
		return $this->data['TS_CREATE'];
	}
	
	/**
	 * Returns the timestamp this lease is due to expire
	 * @return	integer
	 */
	function getExpire() {
		return $this->data['TS_EXPIRE'];
	}
	
	/**
	 * Sets the timestamp this lease is due to expire
	 * @param	integer	Unix timestamp
	 */
	function setExpire($ts) {
		$this->data['TS_EXPIRE'] = $ts;
		$this->_save();
	}
	
	/**
	 * Autosuggest customers callback
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
	 * Creates a new issue and redirects to the edit view of that issue
	 */
	function _view_newIssue() {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (null === ($inventory = Module::getInstance('inventory')) ||
			null === ($project = $inventory->getRepairProject()))
			return null;

		$issue = $project->appendChild($project->_storage->factory('repair'));
		$issue->setCustomer($this->getCustomer());

		return array('redirect' => tpl_view('inventory','edit',$issue->nodeId));
	}
	
	/**
	 * Creates a new invoice and redirects to the edit view of that invoice
	 */
	function _view_newInvoice() {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		
		$project = Module::getInstance('inventory')->getInvoiceProject();
		$invoice = $project->appendChild($project->_storage->factory('invoice'));
		$invoice->setLease($this);

		// Append leasing relations
		foreach (array_keys($leasings = $this->getLeasings()) as $key)
			$invoice->appendChild($leasings[$key]);

		return array('redirect' => tpl_view('inventory','edit',$invoice->nodeId));
	}
	
	/**
	 * @deprecated
	 */
	function _view_newTerminationInvoice() {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (null === ($inventory = Module::getInstance('inventory')) ||
			null === ($project = $inventory->getRepairProject()))
			return null;
		
		$invoice = $project->appendChild($project->_storage->factory('termination_invoice'));
		$invoice->setLease($this);
		
		// Append leasing relations and calculate suggested amount
		$amount = 0;
		foreach (array_keys($leasings = $this->getLeasings()) as $key) {
			$invoice->appendChild($leasings[$key]);
			$item = $leasings[$key]->getItem();
			$amount += $item->getDepreciationCost(time());
		}
		
		$invoice->getUntaxedAmount($amount);

		return array('redirect' => tpl_view('inventory','edit',$invoice->nodeId));
	}

	function _entity_onpreflush() {
		parent::_entity_onpreflush();
		global $synd_user;
		if (!$synd_user->isNull())
			$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_UPDATE'] = time();
	}

	/**
	 * Syncronize assigned SLAs
	 */
	function _entity_onpostflush() {
		global $synd_user;
		parent::_entity_onpostflush();
		
		if (null !== $this->_descriptions) {
			$sql = "
				SELECT ls.sld_node_id FROM synd_inv_lease_sld ls
				WHERE 
					ls.lease_node_id = ".$this->_db->quote($this->nodeId)." AND
					ls.ts_terminated IS NULL";
			$descriptions = (array)$this->_db->getCol($sql);
			
			if (count($remove = array_diff($descriptions, $this->_descriptions))) {
				$sql = "
					UPDATE synd_inv_lease_sld
					SET ts_terminated = ".time()."
					WHERE 
						lease_node_id = ".$this->_db->quote($this->nodeId)." AND
						sld_node_id IN (".implode(',',$this->_db->quote($remove)).") AND
						ts_terminated IS NULL";
				if (SyndLib::isError($this->_db->query($sql)))
					return false;
			}

			if (count($append = array_diff($this->_descriptions, $descriptions))) {
				foreach ($append as $id) {
					$sql = "
						INSERT INTO synd_inv_lease_sld
						(lease_node_id, sld_node_id, create_node_id, ts_create)
						VALUES
						(".$this->_db->quote($this->nodeId).",
						 ".$this->_db->quote($id).",
						 ".$this->_db->quote($synd_user->nodeId).",
						 ".time().")";
					if (SyndLib::isError($this->_db->query($sql)))
						return false;
				}
			}
		}
	}
}

