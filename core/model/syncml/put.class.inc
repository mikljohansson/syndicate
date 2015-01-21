<?php
require_once 'core/model/syncml/command.class.inc';

class synd_syncml_put extends synd_syncml_command {
	function synd_syncml_put($name = 'Put') {
		parent::synd_syncml_command($name);
	}
	
	function getPrecedence() {
		return 10;
	}
	
	function getResponse($message, $response) {
		$persistent = $message->getPersistentSession();
		
		foreach (array_keys($items = $this->getElements('Item')) as $key) {
			$data = $items[$key]->getElement('Data');
			$locuri = $items[$key]->selectSingleNode('Source', 'LocURI');
			$collection = $message->getCollection($locuri->getContent());

			if (false === ($instance = $collection->append($data->getMemento())))
				return $this->_status($message, SYNCML_STATUS_COMMAND_FAILED);
			
			$persistent->setGlobalId($collection, $locuri->getContent(), $instance->id());
		}
		
		return $this->_status($message, SYNCML_STATUS_OK);
	}
}
