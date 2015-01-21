<?php
require_once 'core/model/syncml/node.class.inc';

define('SYNCML_STATUS_OK', 200);
define('SYNCML_STATUS_ITEM_ADDED', 201);
define('SYNCML_STATUS_ITEM_NOT_DELETED', 211);
define('SYNCML_STATUS_AUTHENTICATION_ACCEPTED', 212);
define('SYNCML_STATUS_INVALID_CREDENTIALS', 401);
define('SYNCML_STATUS_FORBIDDEN', 403);
define('SYNCML_STATUS_NOT_FOUND', 404);
define('SYNCML_STATUS_CREDENTIALS_MISSING', 407);
define('SYNCML_STATUS_INCOMPLETE_COMMAND', 412);
define('SYNCML_STATUS_COMMAND_FAILED', 500);
define('SYNCML_STATUS_COMMAND_NOT_IMPLEMENTED', 501);
define('SYNCML_STATUS_REFRESH_REQUIRED', 508);
define('SYNCML_STATUS_SERVER_FAILURE', 511);

class synd_syncml_status extends synd_syncml_node {
	function synd_syncml_status($name = 'Status') {
		parent::synd_syncml_node($name);
	}

	function getPrecedence() {
		return 20;
	}
	
	function getStatusCode() {
		$data = $this->getElement('Data');
		return $data->getContent();
	}

	function getResponse($message, $response) {
		$msgref = $this->getElement('MsgRef');
		$cmdref = $this->getElement('CmdRef');

		$session = $message->getSession();
		$command = $session->getCommand($msgref->getContent(), $cmdref->getContent());
		$session->removeCommand($msgref->getContent(), $cmdref->getContent());

		return $command->getStatusResponse($message, $response, $this);
	}
	
	/**
	 * Adds response Status listener 
	 * @param	synd_syncml_message	The message the command is sent in
	 * @param	synd_syncml_command	The command listening for the response Status
	 */
	static function addStatusListener($message, $command) {
		$session = $message->getSession();
		$session->setCommand($message->getMessageId(), $command->getCommandId(), $command);
	}
}
