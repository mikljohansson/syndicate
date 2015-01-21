<?php
require_once 'core/model/syncml/command.class.inc';

class synd_syncml_delete extends synd_syncml_command {
	var $_collection = null;
	
	function synd_syncml_delete($name = 'Delete') {
		parent::synd_syncml_command($name);
	}
	
	static function createDelete($message, $collection, $luid) {
		$delete = synd_syncml_node::factory('Delete');
		$delete->appendChild(synd_syncml_node::factory('CmdID', $message->nextCommandId()));
		$delete->_collection = $collection->getName();

		$item = $delete->appendChild(synd_syncml_node::factory('Item'));
		$target = $item->appendChild(synd_syncml_node::factory('Target'));
		$target->appendChild(synd_syncml_node::factory('LocURI', $luid));

		return $delete;
	}
	
	function getSyncronizeResponse($message, $response, $collection) {
		$persistent = $message->getPersistentSession();
		$session = $message->getSession();

		foreach (array_keys($items = $this->getElements('Item')) as $key) {
			$data = $items[$key]->getElement('Data');
			$type = $this->selectSingleNode('Meta', 'Type');

			$locuri = $items[$key]->selectSingleNode('Source', 'LocURI');
			$id = $persistent->getGlobalId($collection, $locuri->getContent());

			if (null == ($instance = $collection->getInstance($id)))
				return $this->_status($message, SYNCML_STATUS_ITEM_NOT_DELETED);
			if (!$instance->hasPermission($session->getAuthenticatedUser(), 'write'))
				return $this->_failAuthorizationStatus($message);
			if (false === ($instance = $collection->remove($instance)))
				return $this->_status($message, SYNCML_STATUS_COMMAND_FAILED);

			$persistent->removeGlobalId($collection, $instance->id());
		}
		
		return $this->_status($message, SYNCML_STATUS_OK);
	}
	
	function getStatusResponse($message, $response, $status) {
		if ('2' == substr($status->getStatusCode(),0,1) || '404' == $status->getStatusCode()) {
			$persistent = $message->getPersistentSession();
			$collection = $message->getCollection($this->_collection);
			
			foreach (array_keys($items = $this->getElements('Item')) as $key) {
				$locuri = $items[$key]->selectSingleNode('Target', 'LocURI');
				$persistent->removeGlobalId($collection, 
					$persistent->getGlobalId($collection, $locuri->getContent()));
			}
		}
		return synd_syncml_null::instance();
	}	
}

