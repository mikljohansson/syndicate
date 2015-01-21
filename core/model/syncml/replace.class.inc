<?php
require_once 'core/model/syncml/command.class.inc';

class synd_syncml_replace extends synd_syncml_command {
	function synd_syncml_replace($name = 'Replace') {
		parent::synd_syncml_command($name);
	}
	
	function getSyncronizeResponse($message, $response, $collection) {
		$persistent = $message->getPersistentSession();
		$session = $message->getSession();
		$code = SYNCML_STATUS_OK;
		
		foreach (array_keys($items = $this->getElements('Item')) as $key) {
			$data = $items[$key]->getElement('Data');
			$type = $this->selectSingleNode('Meta', 'Type');
			
			$locuri = $items[$key]->selectSingleNode('Source', 'LocURI');
			$guid = $persistent->getGlobalId($collection, $locuri->getContent());
			
			if (null !== ($instance = $collection->getInstance($guid))) {
				if (!$instance->hasPermission($session->getAuthenticatedUser(), 'write'))
					return $this->_failAuthorizationStatus($message);
				if (!$instance->setMemento($data->getMemento($type->getContent())))
					return $this->_status($message, SYNCML_STATUS_COMMAND_FAILED);
			}
			else {
				if (false === ($instance = $collection->append($data->getMemento($type->getContent()))))
					return $this->_status($message, SYNCML_STATUS_COMMAND_FAILED);
				$code = SYNCML_STATUS_ITEM_ADDED;
			}

			$persistent->removeGlobalId($collection, $instance->id());
			$persistent->setGlobalId($collection, $locuri->getContent(), $instance->id());
		}
		
		return $this->_status($message, $code);
	}
}
