<?php
require_once 'unit/SyndNodeTestCase.class.inc';

class _model_Node_Interface extends SyndNodeTestCase {
	function testEncodeIP() {
		$interface = SyndNodeLib::factory('interface');
		$interface->setIpAddress('192.168.1.1');
		$expected = 'c0a80101';
		$this->assertEquals($expected, dechex($interface->data['INFO_IP_ENCODED']));
		
		$interface = SyndNodeLib::factory('interface');
		$interface->merge(array('INFO_IP_ADDRESS' => '192.168.1.1'));
		$expected = 3232235777;
		$this->assertEquals($expected, $interface->data['INFO_IP_ENCODED']);
	}
}