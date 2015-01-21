<?php
require_once 'core/model/node/invoice.class.inc';

/**
 * @deprecated
 */
class synd_node_termination_invoice extends synd_node_invoice {
	function setPaid($ts = null) {
		if (null !== ($lease = $this->getLease())) {
			$items = $lease->getItems();

			// Inactivate all active leasings
			foreach (array_keys($items) as $key) {
				$items[$key]->setCustomer(SyndNodeLib::getInstance('null.null'));
				$items[$key]->save();
			}

			// Move lease and items
			if (null !== ($inventory = Module::getInstance('inventory'))) {
				if (null !== ($sold = $inventory->getSoldFolder())) {
					foreach (array_keys($items) as $key) {
						$sold->appendChild($items[$key]);
						$items[$key]->save();
					}
				}
				if (null !== ($terminated = $inventory->getTerminatedLeaseFolder()))
					$terminated->appendChild($lease);
			}

			$lease->terminate();
			$lease->save();
		}
		
		parent::setPaid($ts);
	}
}
