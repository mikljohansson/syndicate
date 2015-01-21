<?php
require_once 'core/model/syncml/command.class.inc';

class synd_syncml_synchdr extends synd_syncml_command {
	function synd_syncml_synchdr($name = 'SyncHdr') {
		parent::synd_syncml_node($name);
	}
	
	function getPrecedence() {
		return 25;
	}
	
	function getCommandId() {
		return '0';
	}

	function getResponse($message, $response) {
		// Check for supplied credentials and authenticate
		$cred = $this->getElement('Cred');
		if (!$cred->isNull()) 
			return $cred->getResponse($message, $response, $this);
		
		// Return 'Credentials Missing' status
		$locname = $this->selectSingleNode('Source', 'LocName');
		$session = $message->getSession();
		$user = $session->getAuthenticatedUser();
		if (!$locname->isNull() && $user->isNull()) 
			return $this->_failAuthorizationStatus($message);

		return $this->_status($message, SYNCML_STATUS_OK);
	}

	function getTemplateResponse($message) {
		$header = new synd_syncml_synchdr();
		$header->appendChild(new synd_syncml_node('VerDTD', '1.1'));
		$header->appendChild(new synd_syncml_node('VerProto', 'SyncML/1.1'));
		$header->appendChild($this->getElement('SessionID'));
		
		$msgid = $this->getElement('MsgID');
		$session = $message->getSession();
		$header->appendChild(new synd_syncml_node('MsgID', $session->nextMessageId()));
				
		$source = $this->getElement('Source');
		$header->appendChild($source->clone('Target'));
			
		$target = $this->getElement('Target');
		$header->appendChild($target->clone('Source'));

		return $header;
	}
}

