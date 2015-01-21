<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_Node_Computer extends PHPUnit2_Framework_TestCase {
	function testInputValidation() {
		$nic = SyndNodeLib::factory('nic');
		$errors = $nic->validate(array('INFO_MAC_ADDRESS' => '01:02:03:04:05:AG'));
		$this->assertTrue(isset($errors['INFO_MAC_ADDRESS']));

		$errors = $nic->validate(array('INFO_MAC_ADDRESS' => '01:02:03:04:05'));
		$this->assertTrue(isset($errors['INFO_MAC_ADDRESS']));
		
		$nic->merge(array('INFO_MAC_ADDRESS' => '0102030405ab'));
		$this->assertEquals('01:02:03:04:05:AB', $nic->getMacAddress());
		
		
		$interface = SyndNodeLib::factory('interface');
		$errors = $interface->validate(array('INFO_IP_ADDRESS' => '10.0.0.256', 'INFO_HOSTNAME' => 'a bc'));
		$this->assertTrue(isset($errors['INFO_IP_ADDRESS']));
		$this->assertTrue(isset($errors['INFO_HOSTNAME']));

		$interface->merge(array('INFO_HOSTNAME' => 'HOST.example.com'));
		$this->assertEquals('host.example.com', $interface->getHostname());
	}
	
	function testNetworkInterfaces() {
		$computer = SyndNodeLib::factory('computer');
		$nic = $computer->appendChild($computer->_storage->factory('nic'));
		
		$nics = $computer->getNetworkInterfaces();
		$this->assertEquals(1, count($nics));
	}
}