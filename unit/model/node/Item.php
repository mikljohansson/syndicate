<?php
require_once 'unit/SyndNodeTestCase.class.inc';
require_once 'core/lib/rpc/RpcTransport.class.inc';

class _model_Node_Item extends SyndNodeTestCase {
	function setCustomer() {
		$user = SyndNodeLib::factory('user');

		// Test regular item
		$node = SyndNodeLib::factory('item');
		$node->setCustomer(SyndNodeLib::factory('user'));
		$client = $node->getCustomer();
		$this->assertTrue($client->hasPermission($user, 'remove', $node));
		
		// Test leased item
		$node->setCustomer(SyndNodeLib::factory('lease'));
		$client = $node->getCustomer();
		$this->assertFalse($client->hasPermission($user, 'remove', $node));
	}
	
	function testRemote() {
		global $synd_user;
		$node = SyndNodeLib::factory('item');
		$node->save();
		$node->flush();
		
		$xmlrpc = Module::getInstance('xmlrpc');
		$remote = Activator::getInstance($xmlrpc->getEndpoint($node));
		
		$this->assertFalse(empty($node->nodeId));
		$this->assertEquals($node->nodeId, $remote->nodeId);
		$this->assertEquals($node->nodeId(), $remote->nodeId());
	}
	
	function testDepreciationCost() {
		$expected = 15000;

		$node = SyndNodeLib::factory('item');
		$node->data['TS_DELIVERY'] = time();
		$node->data['INFO_COST'] = $expected;
		$actual = $node->getDepreciationCost();
		$this->assertEquals($expected, $actual);
	}
}
