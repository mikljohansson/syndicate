<?php
require_once 'core/model/node/repair.class.inc';

class synd_node_replace extends synd_node_repair {
	function toString() {
		if ($this->isNew())
			return SyndLib::translate('Replacement of faulty item');
		return parent::toString();
	}
	
	function createRelation() {
		return $this->_storage->factory('replaced');
	}

	function getReplacements() {
		$replacements = array();
		foreach (array_keys($relations = $this->getRelations()) as $key) {
			$replacement = $relations[$key]->getReplacement();
			if (!$replacement->isNull())
				$replacements[] = $replacement;
		}
		return $replacements;
	}

	/**
	 * Moves items to 'back from repairs' folder when closed
	 */
	function setStatus($status, $timestamp = null) {
		parent::setStatus($status, $timestamp);
		
		if ($status >= self::CLOSED) {
			$inventory = Module::getInstance('inventory');
			$repaired = $inventory->getRepairedFolder();

			if (null != $repaired) {
				foreach (array_keys($items = $this->getItems()) as $key) {
					$repaired->appendChild($items[$key]);
					$items[$key]->save();
				}
			}
		}
	}

	function getReceiptFilter() {
		$values = array(
			'REPLACEMENT' => $this->_itemReceiptFilter($this->getReplacements()),
			);
		return array_merge(parent::getReceiptFilter(), $values);
	}

	function validateFlow(&$data, &$errors, $step) {
		if ('flow/step1.tpl' == $step) {
			if (empty($data['replacement']))
				return false;
		}
		return parent::validateFlow($data, $errors, $step);
	}

	function mergeFlow($data, $step) {
		if (!empty($data['item'])) {
			$relation = $this->appendItem(SyndNodeLib::getInstance($data['item']));
			if (!empty($data['replacement']))
				$relation->setReplacement(SyndNodeLib::getInstance($data['replacement']));
			unset($data['item'], $data['replacement']);
		}

		if ('flow/step2.tpl' == $step && !empty($data['confirm']) && $this->isNew()) {
			$inventory = Module::getInstance('inventory');

			// Switch clients and folders
			foreach (array_keys($relations = $this->getRelations()) as $key) {
				$item = $relations[$key]->getItem();
				$item->setCustomer(SyndNodeLib::getInstance('null.null'));
				$item->save();
				
				$replacement = $relations[$key]->getReplacement();
				if (!$replacement->isNull()) {
					$replacement->setCustomer($this->getCustomer());
					$replacement->setFolder($item->getFolder());
					$replacement->save();
				}
			}
			
			// Move replaced items to repair folder
			if (null != ($folder = $inventory->getRepairFolder())) {
				foreach (array_keys($items = $this->getItems()) as $key)
					$folder->appendChild($items[$key]);
			}

			$this->save();
		}
		
		return parent::mergeFlow($data, $step);
	}

	function merge($data) {
		if (!empty($data['item'])) {
			$relation = $this->appendItem(SyndNodeLib::getInstance($data['item']));
			$relation->setReplacement(SyndNodeLib::getInstance($data['replacement']));
			unset($data['item'], $data['replacement']);
		}
		parent::merge($data);
	}
	
	function isValid() {
		$client = $this->getCustomer();
		// Check that items no longer belong to client
		foreach (array_keys($items = $this->getItems()) as $key) {
			$itemClient = $items[$key]->getCustomer();
			if ($itemClient->nodeId == $client->nodeId)
				return false;
		}

		// Check that replacements now belong to client
		foreach (array_keys($replacements = $this->getReplacements()) as $key) {
			$itemClient = $replacements[$key]->getCustomer();
			if ($itemClient->nodeId != $client->nodeId)
				return false;
		}
		
		return true;
	}
}
