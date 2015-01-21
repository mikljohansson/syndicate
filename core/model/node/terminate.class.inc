<?php
require_once 'core/model/node/repair.class.inc';

/**
 * Lease termination issue
 */
class synd_node_terminate extends synd_node_repair {
	function toString() {
		if ($this->isNew())
			return SyndLib::translate('Terminate a lease');
		return parent::toString();
	}
	
	/**
	 * Traps append of items and creates relations
	 */
	function appendChild($child) {
		if (($child instanceof synd_node_item)) {
			$relation = $this->appendChild($this->_storage->factory('terminated'));
			$relation->setChild($child->getLeasing());
			return $relation;
		}
		return parent::appendChild($child);
	}

	function _findLeases($string) {
		$inventory = Module::getInstance('inventory');
		return $inventory->findLeases($string);
	}

	function validateFlow(&$data, &$errors, $step) {
		if ('flow/step1.tpl' == $step)
			return null != $data['CLIENT_NODE_ID'];
		return parent::validateFlow($data, $errors, $step);
	}

	function mergeFlow($data, $step) {
		if (isset($data['CLIENT_NODE_ID'])) {
			$this->setLease(SyndNodeLib::getInstance($data['CLIENT_NODE_ID']));
			unset($data['CLIENT_NODE_ID']);
		}

		if ('flow/step2.tpl' == $step && !empty($data['confirm']) && $this->isNew()) {
			$inventory = Module::getInstance('inventory');
			if (empty($data['folder']))
				$itemFolder = $inventory->getTerminateFolder();
			else
				$itemFolder = SyndNodeLib::getInstance($data['folder']);

			$lease = $this->getLease();
			
			// Terminate leasings
			foreach (array_keys($leasings = $this->getUnreturnedLeasings()) as $key) {
				if (!$leasings[$key]->terminate()) 
					trigger_error("Could not terminate lease '$lease->nodeId'", E_USER_ERROR);
				if (null != $itemFolder) {
					$item = $leasings[$key]->getItem();
					$itemFolder->appendChild($item);
					$item->save();
				}
			}
			
			// Terminate the lease itself
			if (!$lease->terminate())
				trigger_error("Could not terminate lease '$lease->nodeId'", E_USER_ERROR);
			else if (null != ($terminated = $inventory->getTerminatedLeaseFolder())) {
				$terminated->appendChild($lease);
				$lease->save();
			}
		}

		return parent::mergeFlow($data, $step);
	}

	function createRelation() {
		return SyndNodeLib::factory('terminated');
	}

	function getLease() {
		return $this->getCustomer();
	}
	
	function setLease($lease) {
		$this->setCustomer($lease);
		$this->data['INFO_HEAD'] = SyndLib::translate('Lease terminated', $lease->toString());
		foreach (array_keys($leasings = $lease->getLeasings()) as $key)
			$this->appendItem($leasings[$key]);
	}
	
	function getRelations() {
		if (null === $this->_relations) {
			$sql = "
				SELECT r.node_id FROM synd_relation r
				WHERE r.parent_node_id = ".$this->_db->quote($this->nodeId);
			$this->_relations = (array)$this->_db->getCol($sql);
			$this->_node_onchange_internal();
		}
		return $this->_storage->getInstances($this->_relations);
	}
	
	function getReturnedLeasings() {
		$leasings = array();
		foreach (array_keys($relations = $this->getRelations()) as $key)
			$leasings[$key] = $relations[$key]->getRelation();
		return SyndLib::filter($leasings, 'isInactive');
	}

	function getUnreturnedLeasings() {
		$leasings = array();
		foreach (array_keys($relations = $this->getRelations()) as $key)
			$leasings[$key] = $relations[$key]->getRelation();
		return SyndLib::filter($leasings, 'isActive');
	}

	function isRepair() {
		return !$this->isClosed() || !$this->isWarrantyIssue();
	}
	
	/**
	 * Inactivates all coupled leasings when closed
	 */
	function setStatus($status, $timestamp = null) {
		parent::setStatus($status, $timestamp);
		if ($status >= self::CLOSED)
			SyndLib::invoke($this->getUnreturnedLeasings(), 'setInactive');
	}
}
