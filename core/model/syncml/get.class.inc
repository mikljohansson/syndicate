<?php
require_once 'core/model/syncml/command.class.inc';

class synd_syncml_get extends synd_syncml_command {
	function synd_syncml_get($name = 'Get') {
		parent::synd_syncml_command($name);
	}
	
	function getResponse($message, $response) {
		$results = synd_syncml_node::factory('Results');
		$results->appendChild(new synd_syncml_node('CmdID', $message->nextCommandId()));
		$results->appendChild(new synd_syncml_node('MsgRef', $message->getMessageId()));
		$results->appendChild(new synd_syncml_node('CmdRef', $this->getCommandId()));
		
		$meta = $this->getElement('Meta');
		$results->appendChild(clone $meta);
		
		foreach (array_keys($items = $this->getElements('Item')) as $key) {
			$locuri = $items[$key]->selectSingleNode('Target', 'LocURI');
			$collection = $message->getCollection($locuri->getContent());
			if ($collection->isNull())
				return $this->_status($message, SYNCML_STATUS_NOT_FOUND);

			foreach (array_keys($contents = $collection->getContents($message)) as $key) {
				$item = $results->appendChild($items[$key]->getTemplateResponse($message));
				$data = $item->appendChild(synd_syncml_node::factory('Data'));
				$data->appendChild($contents[$key]);
			}
		}
		
		$document = $response->getDocument();
		$body = $document->getElement('SyncBody');
		$body->appendChild($results);
		
		return $this->_status($message, SYNCML_STATUS_OK);
	}
}
