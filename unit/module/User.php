<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/lib/rpc/RpcTransport.class.inc';

class _modules_User extends PHPUnit2_Framework_TestCase {
	var $_path = null;
	var $_user = null;
	var $_password = null;
	
	function setUp() {
		SyndNodeLib::loadClass('user');
		if (null != ($user = synd_node_user::resolveLogin('_unit_test'))) {
			$user->delete();
			$user->flush();
		}
		
		$this->_user = SyndNodeLib::factory('user');
		$this->_user->setUsername('_unit_test');
		$this->_user->setPassword($this->_password = md5(uniqid('')));
		
		$this->_user->save();
		$this->_user->_storage->flush();

		$this->_path = substr(dirname(__FILE__), strlen(rtrim($_SERVER['DOCUMENT_ROOT'],DIRECTORY_SEPARATOR)));
	}
	
	function tearDown() {
		$this->_user->delete();
		$this->_user->_storage->flush();
	}

	function getActivator() {
		return new RpcModuleActivator(Module::getInstance('rpc'), 'php');
	}

	function testValidateUser() {
		$module = Module::getInstance('user');
		$actual = $module->validateLogin('_unit_test', $this->_password);
		$this->assertEquals($this->_user->nodeId, $actual->nodeId);
	}

	function testRemoteAuth() {
		require_once 'core/lib/rpc/RpcTransport.class.inc';
		$node = SyndNodeLib::factory('unit_test');
		$node->save();
		$node->flush();
				
		// Test Basic authentication
		$endpoint = str_replace('://','://'.$this->_user->getLogin().':'.$this->_password.'@', $this->getActivator()->getEndpoint($node));
		$transport = Activator::getTransport($endpoint);
		$actual = $transport->call('getAuthenticatedUser');
		$this->assertEquals($this->_user->id(), $actual);
		
		$node->delete();
	}

	function testSuggestedUsers() {
		$module = Module::getInstance('user');
		$user = SyndNodeLib::factory('user');
		$user->data['USERNAME'] = md5(uniqid(''));
		$user->data['PASSWORD'] = md5(uniqid(''));
		$user->data['INFO_HEAD'] = 'Mikael Johansson';
		$user->data['INFO_EMAIL'] = $user->data['USERNAME'].'@example.com';
		$user->save();
		$user->flush();
		
		$actual = $module->findSuggestedUsers(substr($user->data['USERNAME'],0,-4));
		$expected = array($user->getEmail());
		$this->assertEquals($expected, array_keys($actual));
		
		$user->delete();
	}
}
