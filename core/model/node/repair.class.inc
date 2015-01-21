<?php
require_once 'core/model/node/issue.class.inc';

/**
 * Items repair issue
 */
class synd_node_repair extends synd_node_issue {
	var $_relations = null;
	
	function __factory() {
		global $synd_user;
		parent::__factory();

		$inventory = Module::getInstance('inventory');
		$this->setParent($inventory->getRepairProject());

		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['ASSIGNED_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_RESOLVE_BY'] = strtotime('+3 months');
	}

	static function getEntityQuery(DomainStorage $storage) {
		$query = parent::getEntityQuery($storage);
		$query->join('synd_issue_repair');
		return $query;
	}
	
	function toString() {
		if ($this->isNew())
			return SyndLib::translate('Recieve item for repair');
		return parent::toString();
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('issue_client' == $perm && ($this->getCustomer() instanceof synd_node_lease))
			return false;
		if ('read' == $perm && in_array($arg, array(
			'flow/step1.tpl','flow/step2.tpl','flow/step3.tpl')))
			return true;
		if ('merge' == $perm && 'FLAG_NO_WARRANTY' == $arg)
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}
	
	function isNonWarrantyIssue() {
		return !empty($this->data['FLAG_NO_WARRANTY']);
	}

	function isWarrantyIssue() {
		return empty($this->data['FLAG_NO_WARRANTY']);
	}

	function createRelation() {
		return $this->_storage->factory('repaired');
	}
	
	function getRelations() {
		if (null === $this->_relations) {
			$sql = "
				SELECT r.node_id FROM synd_relation r
				WHERE r.child_node_id = ".$this->_db->quote($this->nodeId);
			$this->_relations = (array)$this->_db->getCol($sql);
		}
		return $this->_storage->getInstances($this->_relations);
	}

	function getItems() {
		$items = array();
		foreach (array_keys($relations = $this->getRelations()) as $key)
			$items[] = $relations[$key]->getItem();
		return $items;
	}
	
	function appendChild($child) {
		if (($child instanceof synd_node_relation)) {
			if (null === $this->_relations)
				$this->getRelations();
			if (!in_array($child->nodeId, $this->_relations))
				$this->_relations[] = $child->nodeId;
		}
		if (($child instanceof synd_node_item))
			return $this->appendItem($child);
		return parent::appendChild($child);
	}
	
	function appendItem($item) {
		foreach (array_keys($items = $this->getItems()) as $key) {
			if ($items[$key]->nodeId == $item->nodeId)
				return null;
		}
		
		$relation = $this->appendChild($this->createRelation());
		$relation->setIssue($this);
		$relation->setItem($item);
		
		$client = $item->getCustomer();
		if (!$client->isNull())
			$this->setCustomer($client);
			
		return $relation;
	}

	function removeItem($item) {
		foreach (array_keys($relations = $this->getRelations()) as $key) {
			$relationItem = $relations[$key]->getItem();
			if ($relationItem->nodeId == $item->nodeId) {
				$relations[$key]->delete();
				unset($this->_relations[array_search($relations[$key]->nodeId,$this->_relations)]);
			}
		}
	}

	/**
	 * Moves items to 'back from repairs' folder when closed
	 */
	function setStatus($status, $timestamp = null) {
		parent::setStatus($status, $timestamp);

		if ($status >= self::CLOSED) {
			$inventory = Module::getInstance('inventory');
			$repair = $inventory->getRepairFolder();
			$repaired = $inventory->getRepairedFolder();

			if (null != $repaired) {
				foreach (array_keys($items = $this->getItems()) as $key) {
					$parent = $items[$key]->getFolder();
					if ($repair->nodeId == $parent->nodeId) {
						$repaired->appendChild($items[$key]);
						$items[$key]->save();
					}
				}
			}
		}
	}

	function getClientOptions() {
		return SyndLib::invoke($this->getItems(),'getClient');
	}
	
	function save() {
		parent::save();
		SyndLib::invoke($this->getRelations(), 'save');
	}
	
	function delete() {
		parent::delete();
		SyndLib::invoke($this->getRelations(), 'delete');
	}
	
	function toPrinter($printer) {
		$template = $this->getReceiptTemplate();
		if ($template->isNull()) 
			return false;
		require_once 'core/lib/SyndPrint.class.inc';
		return SyndPrint::pdfPrint($printer, $template->path(), $this->getReceiptFilter());
	}

	function printReceipt() {
		if (null != ($printer = $this->getPrinter())) 
			return $this->toPrinter($printer);
		return false;
	}
	
	function getPrinter() {
		if (null !== ($inventory = Module::getInstance('inventory')))
			return $inventory->getPrinter();
		return null;
	}
	
	function getReceiptTemplate() {
		$inventory = Module::getInstance('inventory');
		$templates = $inventory->getInheritedTemplates(get_class($this));
		if (!empty($templates))
			return $templates[key($templates)];
		return $template = SyndNodeLib::getInstance('null.null');
	}

	function getReceiptFilter() {
		$client = $this->getCustomer();
		$assigned = $this->getAssigned();

		$values = array(
			'ID'			=> $this->objectId(), 
			'DATE'			=> date('Y-m-d'),
			'CREATED'		=> date('Y-m-d', $this->data['TS_CREATE']),
			'CUSTOMER'		=> array(
				'CAPTION'		=> null != $client->getContact() ? $client->toString().' ('.$client->getContact().')' : $client->toString(),
				'NAME'			=> $client->toString(),
				'PHONE'			=> $client->getPhone()),
			'ASSIGNED'		=> array(
				'CAPTION'		=> null != $assigned->getContact() ? $assigned->toString().' ('.$assigned->getContact().')' : $assigned->toString(),
				'NAME'			=> $assigned->toString(),
				'PHONE'			=> $assigned->getPhone()),
			'DESCRIPTION'	=> $this->getDescription(), 
			'ITEM'			=> $this->_itemReceiptFilter($this->getItems()),
			);
		
		return $values;
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
	 * Direct link to autogenerated PDF
	 */
	function _view_receipt_pdf(Request $request, Template $page) {
		require_once 'core/lib/SyndPrint.class.inc';
		$receipt = $this->getReceiptTemplate();
		if (!$receipt->isNull()) {
			header('Content-Type: application/pdf');
			header('Content-Disposition: attachment; filename="'.str_replace('"','',$this->getTitle()).'.pdf"');
			return array('content' => SyndPrint::pdfMerge($receipt->path(), $this->getReceiptFilter()), 'page' => null);
		}
	}

	function _findItems($string) {
		$inventory = Module::getInstance('inventory');
		return $inventory->findItems($string);
	}

	function merge($data) {
		if (isset($data['INFO_STATUS']))
			$data['FLAG_NO_WARRANTY'] = (int)!empty($data['FLAG_NO_WARRANTY']);
		parent::merge($data);
	}

	function validateFlow(&$data, &$errors, $step) {
		if (empty($this->data['INFO_HEAD']))
			$data['INFO_HEAD'] = empty($data['content']) ? SyndLib::translate('New repair issue') : tpl_chop($data['content'], 35);
		if ('flow/step1.tpl' == $step) {
			if (empty($data['item']))
				return false;
			if (empty($data['content']) && isset($_REQUEST['post'])) {
				$errors['content']['msg'] = SyndLib::translate('Please provide a repair description');
				return false;
			}
		}
		return parent::validateFlow($data, $errors, $step);
	}

	function mergeFlow($data, $step) {
		if (empty($this->data['INFO_HEAD']))
			$data['INFO_HEAD'] = empty($data['content']) ? SyndLib::translate('New repair issue') : tpl_chop($data['content'], 75);
		if (isset($data['item']) && null != ($item = SyndNodeLib::getInstance($data['item'])))
			$this->appendItem($item);
		if (!empty($data['closed']))
			$this->setStatus(self::CLOSED);

		$result = parent::mergeFlow($data, $step);
		
		if ('flow/step2.tpl' == $step && !empty($data['confirm']) && $this->isNew()) {
			$this->save();
			$this->flush();

			// Move replaced items to selected folder (if any)
			if (isset($data['folder']) && null != ($folder = SyndNodeLib::getInstance($data['folder']))) {
				foreach (array_keys($items = $this->getItems()) as $key)
					$folder->appendChild($items[$key]);
			}

			if (!empty($data['receipt'])) {
				$this->printReceipt();
				return tpl_view('issue', 'flow', $this->nodeId, 3, array('receipt' => 1));
			}
		}
		
		return $result;
	}

	function _view_printReceipt(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		$this->printReceipt();
	}

	/**
	 * Sets the creator and assigned in case the effective user has changed
	 */
	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();
		if ($this->isNew()) {
			$this->setCreator($synd_user);
			$this->setAssigned($synd_user);
		}
	}

	function _entity_onpostinsert() {
		global $synd_user;
		parent::_entity_onpostinsert();
		if (!$this->isWarrantyIssue())
			$this->fireEvent(SyndType::factory('onnonwarranty',$this));
		
		$task = $this->appendChild(SyndNodeLib::factory('task'));
		$content = $task->getContent();
		$content->setContent(SyndLib::translate('Set-up and adjusting time'));

		$task->setCreator($synd_user);
		$task->setDuration(60*30);
		$task->save();
	}
}
