<?php
require_once 'core/model/syncml/command.class.inc';

class synd_syncml_add extends synd_syncml_command {
	function synd_syncml_add($name = 'Add') {
		parent::synd_syncml_command($name);
	}
	
	function getSyncronizeResponse($message, $response, $collection) {
		$persistent = $message->getPersistentSession();
		
		foreach (array_keys($items = $this->getElements('Item')) as $key) {
			$data = $items[$key]->getElement('Data');
			$type = $this->selectSingleNode('Meta', 'Type');

			if (false === ($instance = $collection->append($data->getMemento($type->getContent()))))
				return $this->_status($message, SYNCML_STATUS_COMMAND_FAILED);
			
			$locuri = $items[$key]->selectSingleNode('Source', 'LocURI');
			$persistent->setGlobalId($collection, $locuri->getContent(), $instance->id());
		}
		
		return $this->_status($message, SYNCML_STATUS_ITEM_ADDED);
	}
}
