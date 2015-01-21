<?php
require_once 'unit/SyndNodeTestCase.class.inc';

class _model_Node_VLAN extends SyndNodeTestCase {
	function setUp() {
		require_once 'core/model/node/interface.class.inc';
	}
	
	function testNetworks() {
		$folder = SyndNodeLib::factory('folder');
		$folder->save();
		$node = $folder->appendChild($folder->_storage->factory('vlan'));
		$node->data['INFO_HEAD'] = '_unit_test: This VLAN is safe to delete';

		$expected = array();
		$this->assertEquals($expected, $node->getNetworks());
		
		// Test merge network
		$expected = array(array('INFO_NETWORK_ADDRESS' => '10.0.0.1', 'INFO_NETWORK_MASK' => '255.255.255.0'));
		$node->merge(array('network' => reset($expected)));
		
		$networks = $node->getNetworks();
		$this->assertEquals(1, count($networks));
		if (count($networks)) {
			$this->assertEquals('10.0.0.1', $networks[0]['INFO_NETWORK_ADDRESS']);
			$this->assertEquals('255.255.255.0', $networks[0]['INFO_NETWORK_MASK']);
			$this->assertEquals(hexdec(dechex(
				synd_node_interface::ip2long('10.0.0.1') & 
				synd_node_interface::ip2long('255.255.255.0'))), 
				$networks[0]['INFO_ENCODED_NET']);
			$this->assertEquals(
				synd_node_interface::ip2long('255.255.255.0'), 
				$networks[0]['INFO_ENCODED_MASK']);
		}
		
		// Test flush and reload
		$node->save();
		$node->flush();
		
		$this->assertNotNull($node->_networks);
		$node->_networks = null;
		$networks = $node->getNetworks();
		$this->assertEquals(1, count($networks));
		
		// Test validate ip and netmask
		$data = array('INFO_NETWORK_ADDRESS' => '256.0.0.1', 'INFO_NETWORK_MASK' => '256.255.255.0');
		$errors = $node->validate(array('INFO_HEAD' => 'a bc', 'network' => $data));
		$this->assertTrue(isset($errors['INFO_HEAD']));
		$this->assertTrue(isset($errors['INFO_NETWORK_ADDRESS']));
		
		// Test parent network collision
		$data = array('INFO_NETWORK_ADDRESS' => '10.0.0.1', 'INFO_NETWORK_MASK' => '255.255.0.0');
		$errors = $node->validate(array('INFO_HEAD' => 'a-bc', 'network' => $data));
		$this->assertFalse(isset($errors['INFO_HEAD']));
		$this->assertTrue(isset($errors['INFO_NETWORK_ADDRESS']));
		
		// Test subnetwork collision
		$data = array('INFO_NETWORK_ADDRESS' => '10.0.0.1', 'INFO_NETWORK_MASK' => '255.255.255.192');
		$errors = $node->validate(array('network' => $data));
		$this->assertTrue(isset($errors['INFO_NETWORK_ADDRESS']));

		$node->delete();
		$folder->delete();
	}
}