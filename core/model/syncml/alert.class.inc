<?php
require_once 'core/model/syncml/command.class.inc';
require_once 'core/model/syncml/status.class.inc';
require_once 'core/model/syncml/delete.class.inc';

define('SYNCML_ALERT_NORMAL', 200);
define('SYNCML_ALERT_SLOW', 201);
define('SYNCML_ALERT_ONEWAY_CLIENT', 202);
define('SYNCML_ALERT_REFRESH_CLIENT', 203);
define('SYNCML_ALERT_ONEWAY_SERVER', 204);
define('SYNCML_ALERT_REFRESH_SERVER', 205);
define('SYNCML_ALERT_NEXT_MESSAGE', 222);

class synd_syncml_alert extends synd_syncml_command {
	var $_strategy = null;
	
	function synd_syncml_alert($name = 'Alert') {
		parent::synd_syncml_command($name);
	}
	
	function getStrategy() {
		if (null === $this->_strategy) {
			$data = $this->getElement('Data');
			if (class_exists($class = 'synd_syncml_alert_'.$data->getContent()))
				$this->_strategy = new $class($this);
		}
		return $this->_strategy;
	}

	/**
	 * Replies with a Status and an optional response Alert
	 */
	function getResponse($message, $response) {
		if (null != ($strategy = $this->getStrategy()))
			return $strategy->getResponse($message, $response);
		return parent::getResponse($message, $response);
	}

	/**
	 * Replies with a null and sets the send state
	 */
	function getStatusResponse($message, $response, $status) {
		if (null != ($strategy = $this->getStrategy()))
			return $strategy->getStatusResponse($message, $response, $status);
		return parent::getStatusResponse($message, $response, $status);
	}

	/**
	 * Replies with a Sync
	 */
	function getPackageResponse($message, $response) {
		if (null != ($strategy = $this->getStrategy()))
			return $strategy->getPackageResponse($message, $response);
		return parent::getPackageResponse($message, $response);
	}
}

/**
 * @access		private
 */
class synd_syncml_alert_strategy {
	var $_parent = null;
	var $_sent = array();
	
	function __construct($parent) {
		$this->_parent = $parent;
	}
	
	function isNull() {
		return $this->_parent->isNull();
	}
	
	function getName() {
		return $this->_parent->getName();
	}
	
	function getCommandId() {
		return $this->_parent->getCommandId();
	}
	
	function getElement($name) {
		return $this->_parent->getElement($name);
	}

	function getElements($name) {
		return $this->_parent->getElements($name);
	}
	
	function toTimestamp($date) {
		return strtotime(preg_replace('/(\d{6})T(\d{2})(\d{2})(\d{2})/', '\1 \2:\3:\4', $date));
	}

	function getResponse($message, $response) {
		return $this->_status($message, SYNCML_STATUS_OK);
	}

	function getStatusResponse($message, $response, $status) {
		if ('2' == substr($status->getStatusCode(),0,1)) {
			$session = $message->getSession();
			$session->addPackageCommand($this->_parent);
		}
		return synd_syncml_null::instance();
	}

	/**
	 * @todo	The usage of synd_syncml_message::isOversized() is non-optimal
	 * @todo	The case when a single item causes message oversize should be handled how?
	 * @uses	SyncMLSession::addPackageCommand()
	 */
	function getPackageResponse($message, $response) {
		$persistent = $message->getPersistentSession();
		$session = $message->getSession();

		$document = $response->getDocument();
		$body = $document->getElement('SyncBody');
		
		$devinfo = $persistent->getDeviceInfo();
		$supportNumberOfChanges = $devinfo->getElement('SupportNumberOfChanges');

		$final = true;
		$count = 0;

		foreach (array_keys($items = $this->getElements('Item')) as $key) {
			$locuri = $items[$key]->selectSingleNode('Source', 'LocURI');
			$collection = $message->getCollection($locuri->getContent());

			// Build and append the Sync element
			$contents = $this->getCollectionContents($message, $items[$key], $collection);
			$removed = array_diff($persistent->getMappings($collection), 
				SyndLib::invoke($collection->getContents(), 'id'));

			$sync = $body->appendChild(synd_syncml_node::factory('Sync'));
			$sync->appendChild(synd_syncml_node::factory('CmdID', $message->nextCommandId()));
			$sync->appendChild($items[$key]->getElement('Target'));
			$sync->appendChild($items[$key]->getElement('Source'));
			
			if (!$supportNumberOfChanges->isNull())
				$sync->appendChild(synd_syncml_node::factory('NumberOfChanges', count($contents) + count($removed)));
			
			// Append content mementos (Add/Replace commands)
			foreach (array_keys($contents) as $key2) {
				// Check if command has been sent in a previous message
				if (in_array($contents[$key2]->id(), $this->_sent))
					continue;
				
				// Send Replace if item already exists on client
				if (null !== ($luid = $persistent->getLocalId($collection, $contents[$key2]->id())))
					$command = $sync->appendChild(synd_syncml_node::factory('Replace'));
				else
					$command = $sync->appendChild(synd_syncml_node::factory('Add'));

				$command->appendChild(synd_syncml_node::factory('CmdID', $message->nextCommandId()));
				$meta = $command->appendChild(synd_syncml_node::factory('Meta'));
				$meta->appendChild(synd_syncml_node::factory('Type', 'text/x-vcalendar'));

				$item = $command->appendChild(synd_syncml_node::factory('Item'));
				if (null != $luid) {
					$target = $item->appendChild(synd_syncml_node::factory('Target'));
					$target->appendChild(synd_syncml_node::factory('LocURI', $luid));
				}

				$source = $item->appendChild(synd_syncml_node::factory('Source'));
				$source->appendChild(synd_syncml_node::factory('LocURI', $contents[$key2]->id()));

				$data = $item->appendChild(synd_syncml_node::factory('Data'));
				$data->setMemento($contents[$key2]->getMemento());

				// Check if message is oversize (MaxMsgSize)
				if (!$response->isOversized()) {
					$this->_sent[] = $contents[$key2]->id();
					$count++;
				}
				// If single item is oversized skip
				else if (0 == $count) {
					$this->_sent[] = $contents[$key2]->id();
					$sync->removeLastChild();
				}
				else {
					$sync->removeLastChild();
					$final = false;
					break;
				}
			}

			foreach ($removed as $luid => $guid) {
				// Check if command has been sent in a previous message
				if (in_array($guid, $this->_sent))
					continue;

				$delete = $sync->appendChild(synd_syncml_delete::createDelete($message, $collection, $luid));
				
				// Check if message is oversize (MaxMsgSize)
				if (!$response->isOversized()) {
					synd_syncml_status::addStatusListener($response, $delete);
					$this->_sent[] = $guid;
				}
				// If single item is oversized skip
				else if (0 == $count) {
					$this->_sent[] = $guid;
					$sync->removeLastChild();
				}
				else {
					$sync->removeLastChild();
					$final = false;
					break;
				}
			}

			// Update our saved anchor to reflect the new sync
			$next = $items[$key]->selectSingleNode('Meta', 'Anchor', 'Next');
			$persistent->setLastAnchor($collection, $next->getContent());
		}

		if (!$final)
			$session->addPackageCommand($this->_parent);
		
		return synd_syncml_null::instance();
	}

	function _status($message, $code) {
		$status = $this->_parent->_status($message, $code);
		if ('2' == substr($code,0,1)) {
			foreach (array_keys($items = $this->getElements('Item')) as $key) {
				$meta = $items[$key]->getElement('Meta');
				$anchor = $meta->getElement('Anchor');
				$next = $anchor->getElement('Next');

				if (!$next->isNull()) {
					$item = $status->appendChild(synd_syncml_node::factory('Item'));
					$data = $item->appendChild(synd_syncml_node::factory('Data'));

					$anchor = $data->appendChild(clone $anchor);
					$anchor->removeChildren();
					$anchor->appendChild(clone $next);
				}
			}
		}
		return $status;
	}
}

/**
 * Normal Two-Way sync
 * @access	private
 */
class synd_syncml_alert_200 extends synd_syncml_alert_strategy {
	function getResponse($message, $response) {
		$code = SYNCML_STATUS_OK;
		$persistent = $message->getPersistentSession();
		$session = $message->getSession();
		
		$document = $response->getDocument();
		$body = $document->getElement('SyncBody');
		$items = $this->getElements('Item');

		foreach (array_keys($items) as $key) {
			$locuri = $items[$key]->selectSingleNode('Target', 'LocURI');
			$anchor = $items[$key]->selectSingleNode('Meta', 'Anchor');
			$collection = $message->getCollection($locuri->getContent());
			if ($collection->isNull()) {
				$code = ($collection instanceof SyncableUnauthorizedCollection) ? 
					SYNCML_STATUS_FORBIDDEN : SYNCML_STATUS_NOT_FOUND;
				continue;
			}
			
			// Setup the response Alert
			$alert = $body->appendChild(synd_syncml_node::factory('Alert'));
			$alert->appendChild(new synd_syncml_node('CmdID', $message->nextCommandId()));
			
			// Check Last anchor if we should force a slow sync
			$last = $anchor->getElement('Last');
			if (null == $last->getContent() || $last->getContent() == $persistent->getLastAnchor($collection))
				$alert->appendChild(synd_syncml_node::factory('Data', SYNCML_ALERT_NORMAL));
			else {
				$alert->appendChild(synd_syncml_node::factory('Data', SYNCML_ALERT_SLOW));
				$code = SYNCML_STATUS_REFRESH_REQUIRED;
			}

			$item = $alert->appendChild(synd_syncml_node::factory('Item'));
			$source = $items[$key]->getElement('Source');
			$item->appendChild($source->clone('Target'));
			$target = $items[$key]->getElement('Target');
			$item->appendChild($target->clone('Source'));

			$meta = $items[$key]->getElement('Meta');
			$newMeta = $item->appendChild(clone $meta);
			$newMeta->removeChildren();
			$newAnchor = $newMeta->appendChild(clone $anchor);
			$newAnchor->removeChildren();

			// Send our saved Last anchor if one exists
			$newAnchor->appendChild(new synd_syncml_node('Last', 
				null != $persistent->getLastAnchor($collection) ? 
				$persistent->getLastAnchor($collection) : '00000000T000000Z'));
			$newAnchor->appendChild($anchor->getElement('Next'));

			// Register the outgoing Alert as waiting for response Status
			synd_syncml_status::addStatusListener($response, $alert);
		}
		
		return $this->_status($message, $code);
	}

	/**
	 * Retrive modified content from the collection
	 */
	function getCollectionContents($message, $item, $collection) {
		$last = $item->selectSingleNode('Meta', 'Anchor', 'Last');
		$lastTimestamp = $this->toTimestamp($last->getContent());
		$next = $item->selectSingleNode('Meta', 'Anchor', 'Next');
		$nextTimestamp = $this->toTimestamp($next->getContent());
		return $collection->getContents($lastTimestamp, $nextTimestamp);
	}
}

/**
 * Slow Two-Way sync. 
 * @access	private
 */
class synd_syncml_alert_201 extends synd_syncml_alert_strategy {
	function getResponse($message, $response) {
		$persistent = $message->getPersistentSession();
		$session = $message->getSession();

		$document = $response->getDocument();
		$body = $document->getElement('SyncBody');
		$code = SYNCML_STATUS_OK;

		foreach (array_keys($items = $this->getElements('Item')) as $key) {
			$locuri = $items[$key]->selectSingleNode('Target', 'LocURI');
			$collection = $message->getCollection($locuri->getContent());
			if ($collection->isNull()) {
				$code = ($collection instanceof SyncableUnauthorizedCollection) ? 
					SYNCML_STATUS_FORBIDDEN : SYNCML_STATUS_NOT_FOUND;
				continue;
			}
			
			// Setup the response Alert for Slow-Sync
			$alert = $body->appendChild(synd_syncml_node::factory('Alert'));
			$alert->appendChild(synd_syncml_node::factory('CmdID', $message->nextCommandId()));
			$alert->appendChild(synd_syncml_node::factory('Data', SYNCML_ALERT_SLOW));

			$item = $alert->appendChild(synd_syncml_node::factory('Item'));
			$source = $items[$key]->getElement('Source');
			$item->appendChild($source->clone('Target'));
			$target = $items[$key]->getElement('Target');
			$item->appendChild($target->clone('Source'));

			$meta = $items[$key]->getElement('Meta');
			$anchor = $meta->getElement('Anchor');
			
			$newMeta = $item->appendChild(clone $meta);
			$newMeta->removeChildren();
			$newAnchor = $newMeta->appendChild(clone $anchor);
			$newAnchor->removeChildren();

			// Send our saved Last anchor if one exists
			$locuri = $items[$key]->selectSingleNode('Target', 'LocURI');
			$newAnchor->appendChild(new synd_syncml_node('Last', 
				null != $persistent->getLastAnchor($collection) ? 
				$persistent->getLastAnchor($collection) : '00000000T000000Z'));
			$newAnchor->appendChild($anchor->getElement('Next'));

			// Register the outgoing Alert as waiting for response Status
			synd_syncml_status::addStatusListener($response, $alert);
		}
		
		return $this->_status($message, $code);
	}

	/**
	 * Retrieves all content
	 */
	function getCollectionContents($message, $item, $collection) {
		return $collection->getContents();
	}
}

/**
 * One-Way sync from client. 
 *
 * Client sends all its modifications, server does nothing
 *
 * @access	private
 */
class synd_syncml_alert_202 extends synd_syncml_alert_strategy {
	/**
	 * Update the saved anchors to reflect the new sync
	 */
	function getResponse($message, $response) {
		$persistent = $message->getPersistentSession();
		$code = SYNCML_STATUS_OK;
		
		foreach (array_keys($items = $this->getElements('Item')) as $key) {
			$locuri = $items[$key]->selectSingleNode('Target', 'LocURI');
			$collection = $message->getCollection($locuri->getContent());
			if ($collection->isNull()) {
				$code = ($collection instanceof SyncableUnauthorizedCollection) ? 
					SYNCML_STATUS_FORBIDDEN : SYNCML_STATUS_NOT_FOUND;
				continue;
			}
			
			$next = $items[$key]->selectSingleNode('Meta', 'Anchor', 'Next');
			$persistent->setLastAnchor($collection, $next->getContent());
		}
		
		return $this->_status($message, $code);
	}
}

/**
 * Refresh sync from client. 
 *
 * Client sends all its content, server merges changes and removes
 * content that is not present (sent by) on client. Use with caution.
 *
 * @todo	Not implemented
 * @access	private
 */
class synd_syncml_alert_203 extends synd_syncml_alert_strategy {
	function getResponse($message, $response) {
		return $this->_status($message, SYNCML_STATUS_COMMAND_NOT_IMPLEMENTED);
	}
}

/**
 * One-Way sync from server
 *
 * Client initiates sync, server sends its modifications
 *
 * @access	private
 */
class synd_syncml_alert_204 extends synd_syncml_alert_strategy {
	/**
	 * Retrive modified content from the collection
	 */
	function getCollectionContents($message, $item, $collection) {
		$last = $item->selectSingleNode('Meta', 'Anchor', 'Last');
		$lastTimestamp = $this->toTimestamp($last->getContent());
		$next = $item->selectSingleNode('Meta', 'Anchor', 'Next');
		$nextTimestamp = $this->toTimestamp($next->getContent());
		return $collection->getContents($lastTimestamp, $nextTimestamp);
	}
}

/**
 * Refresh from server
 *
 * Client initiates sync, server sends all its content
 *
 * @access	private
 */
class synd_syncml_alert_205 extends synd_syncml_alert_strategy {
	/**
	 * Retrieves all content
	 */
	function getCollectionContents($message, $item, $collection) {
		return $collection->getContents();
	}
}

/**
 * Result alert, request for syncronization results
 * @access		private
 */
class synd_syncml_alert_221 extends synd_syncml_alert_strategy {
}

/**
 * Next message alert
 * @access		private
 */
class synd_syncml_alert_222 extends synd_syncml_alert_strategy {
}

/**
 * End of data for chunked object note received
 * @access		private
 */
class synd_syncml_alert_223 extends synd_syncml_alert_strategy {
	function getResponse($message, $response) {
		return $this->_status($message, SYNCML_STATUS_SERVER_FAILURE);
	}
}
