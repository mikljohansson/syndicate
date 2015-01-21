<?php
require_once 'core/model/syncml/command.class.inc';

class synd_syncml_map extends synd_syncml_command {
	function synd_syncml_map($name = 'Map') {
		parent::synd_syncml_command($name);
	}
	
	function getResponse($message, $response) {
		$persistent = $message->getPersistentSession();
		$session = $message->getSession();

		$locuri = $this->selectSingleNode('Target', 'LocURI');
		$collection = $message->getCollection($locuri->getContent());
		if ($collection->isNull())
			return $this->_status($message, SYNCML_STATUS_NOT_FOUND);
		
		foreach (array_keys($mapitems = $this->getElements('MapItem')) as $key) {
			$luid = $mapitems[$key]->selectSingleNode('Source', 'LocURI');
			$guid = $mapitems[$key]->selectSingleNode('Target', 'LocURI');

			if (null === ($instance = $collection->getInstance($guid->getContent())))
				return $this->_status($message, SYNCML_STATUS_NOT_FOUND);
			if (!$instance->hasPermission($session->getAuthenticatedUser(), 'read'))
				return $this->_failAuthorizationStatus($message);
			
			$persistent->setGlobalId($collection, $luid->getContent(), $instance->id());
		}
		
		return $this->_status($message, SYNCML_STATUS_OK);
	}
}
