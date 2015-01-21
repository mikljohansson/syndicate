<?php
require_once 'core/model/syncml/node.class.inc';
require_once 'core/model/syncml/status.class.inc';
require_once 'core/model/ISyncable.class.inc';

class synd_syncml_command extends synd_syncml_node {
	function synd_syncml_command($name) {
		parent::synd_syncml_node($name);
	}
	
	function getCommandId() {
		$cmdid = $this->getElement('CmdID');
		return $cmdid->getContent();
	}
	
	function _status($message, $code) {
		$noresp = $this->getElement('NoResp');
		if (!$noresp->isNull()) 
			return synd_syncml_null::instance();

		$status = synd_syncml_node::factory('Status');
		$status->appendChild(synd_syncml_node::factory('CmdID', $message->nextCommandId()));
		$status->appendChild(synd_syncml_node::factory('MsgRef', $message->getMessageId()));
		$status->appendChild(synd_syncml_node::factory('CmdRef', $this->getCommandId()));
		$status->appendChild(synd_syncml_node::factory('Cmd', $this->getName()));
		
		$target = $this->getElement('Target');
		if ($target->isNull() && 1 == count($items = $this->getElements('Item')))		
			$target = $items[key($items)]->getElement('Target');
		$status->appendChild($target->getTemplateResponse($message));

		$source = $this->getElement('Source');
		if ($source->isNull() && 1 == count($items = $this->getElements('Item')))		
			$source = $items[key($items)]->getElement('Source');
		$status->appendChild($source->getTemplateResponse($message));

		// Send Chal if not authenticated
		$session = $message->getSession();
		$user = $session->getAuthenticatedUser();
		
		if (SYNCML_STATUS_CREDENTIALS_MISSING == $code || SYNCML_STATUS_FORBIDDEN == $code && $user->isNull()) {
			$status->appendChild(synd_syncml_node::factory('Data', SYNCML_STATUS_CREDENTIALS_MISSING));
			$chal = $status->appendChild(synd_syncml_node::factory('Chal'));
			$meta = $chal->appendChild(synd_syncml_node::factory('Meta'));
			$meta->appendChild(synd_syncml_node::factory('Format', 'b64'));
			$meta->appendChild(synd_syncml_node::factory('Type', 'syncml:auth-basic'));
		}
		else {
			$status->appendChild(synd_syncml_node::factory('Data', $code));
		}

		return $status;
	}

	function _failAuthorizationStatus($message) {
		return $this->_status($message, SYNCML_STATUS_FORBIDDEN);
	}
}
