<?php
require_once 'core/model/syncml/command.class.inc';

class synd_syncml_sync extends synd_syncml_command {
	function synd_syncml_sync($name = 'Sync') {
		parent::synd_syncml_command($name);
	}
	
	function getPrecedence() {
		return 15;
	}
	
	function getResponse($message, $response) {
		$locuri = $this->selectSingleNode('Target', 'LocURI');
		$collection = $message->getCollection($locuri->getContent());
		
		if ($collection->isNull())
			$result = $this->_status($message, SYNCML_STATUS_NOT_FOUND);
		else {
			$document = $response->getDocument();
			$body = $document->getElement('SyncBody');
			foreach (array_keys($children = $this->getChildren()) as $key)
				$body->appendChild($children[$key]->getSyncronizeResponse($message, $response, $collection));
			$result = $this->_status($message, SYNCML_STATUS_OK);
		}
		
		return $result;
	}
}
