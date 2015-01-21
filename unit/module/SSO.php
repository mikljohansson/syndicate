<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/lib/rpc/RpcTransport.class.inc';

class _modules_SSO extends PHPUnit2_Framework_TestCase {
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
		$this->_user->flush();

		$this->_path = substr(dirname(__FILE__), strlen(rtrim($_SERVER['DOCUMENT_ROOT'],DIRECTORY_SEPARATOR)));
	}
	
	function tearDown() {
		$this->_user->delete();
	}
	
	function _testSingleSignOn() {
		return isset($synd_config['sso']['idp']) &&
			isset($synd_config['sso']['sp'][$_SERVER['SERVER_NAME']]);
	}

	function testSingleSignOn() {
		global $synd_config;
		if (!$this->_testSingleSignOn())
			return;
		
		$idp = reset($synd_config['sso']['idp']);
		$sp = $synd_config['sso']['sp'][$_SERVER['SERVER_NAME']];

		// Sign user into a global session		
		$transport = Activator::getTransport($idp['urn']);
		$uri = $transport->invoke('login', array($_SERVER['SERVER_NAME'],'_unit_test', $this->_password));

		$sid = $transport->invoke('getSessionId', array($this->_user));
		$this->assertFalse(SyndLib::isError($sid));
		$this->assertFalse(empty($sid));
		
		$expected = "{$idp['location']}initialize.css?sid=$sid";
		$this->assertEquals($expected, $uri);
		
		// Perform get on uri to initialize session (cookie) with IdP
		$connection = new HttpConnection($uri);
		$resp = $connection->getRequest($uri);
		$this->assertEquals(200, $connection->getStatus());
		
		// Use session to log into an other site
		$jsLocation = "{$idp['location']}session.js?redirect=".rawurlencode("{$sp['location']}callback.js");
		$js = $connection->getRequest($jsLocation);
		$this->assertEquals(200, $connection->getStatus());
		$this->assertEquals('window.location.reload();', $js);
		
		$module = Module::getInstance('user');
		$instances = $module->getSessionInstances($sid);
		$this->assertEquals(array($_SERVER['SERVER_NAME']), $instances);
		
		//Logout from global session
		$css = $connection->getRequest("{$idp['location']}logout.css");
		$this->assertEquals("@import \"{$sp['location']}logout\";\n", $css);
		$this->assertEquals(null, 
			$transport->invoke('getSessionUser', array($sid)));
	}
	
	function testRegister() {
		if (!$this->_testSingleSignOn())
			return;

		$module = Module::getInstance('user');
		$this->assertNull($module->getSessionId($this->_user));
		$module->ssoRegister($_SERVER['SERVER_NAME'], $this->_user);
		$this->assertEquals(32, strlen($module->getSessionId($this->_user)));
	}

	function testRegisterSession() {
		global $synd_config;
		if (!$this->_testSingleSignOn())
			return;

		$module = Module::getInstance('user');
		$this->assertNull($module->getSessionId($this->_user));
		
		$idp = reset($synd_config['sso']['idp']);
		$transport = Activator::getTransport($idp['urn']);
		$uri = $transport->invoke('register', array($_SERVER['SERVER_NAME'], $this->_user->getLogin()));
		$this->assertTrue($uri);
		
		$sid = $module->getSessionId($this->_user);
		$this->assertFalse(SyndLib::isError($sid));
		$this->assertFalse(empty($sid));
		$this->assertTrue(false !== strpos($uri, $sid));

		$location = "$uri?redirect=".urlencode("http://{$_SERVER['SERVER_NAME']}{$this->_path}/SSO.html");
		$connection = new HttpConnection($location);

		$response = $connection->getRequest($location);
		$this->assertEquals(200, $connection->getStatus());
		$this->assertEquals('valid', $response);
	}
}

