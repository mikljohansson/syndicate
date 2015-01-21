<?php
require_once 'core/model/syncml/node.class.inc';
require_once 'core/model/syncml/status.class.inc';

class synd_syncml_cred extends synd_syncml_node {
	function synd_syncml_cred($name = 'Cred') {
		parent::synd_syncml_node($name);
	}
	
	function getResponse($message, $response, $command) {
		$type = $this->selectSingleNode('Meta', 'Type');
		
		switch (strtolower($type->getContent())) {
			case 'syncml:auth-basic':
				$data = $this->getElement('Data');
				$creds = explode(':', base64_decode($data->getContent()), 2);

				$module = Module::getInstance('user');
				if (2 != count($creds) || false === ($user = $module->validateLogin($creds[0], $creds[1])))
					return $command->_status($message, SYNCML_STATUS_INVALID_CREDENTIALS);
				
				$session = $message->getSession();
				$session->setAuthenticatedUser($user);
				
				return $command->_status($message, SYNCML_STATUS_AUTHENTICATION_ACCEPTED);
		}
		
		return parent::getResponse($message, $response);
	}
}
