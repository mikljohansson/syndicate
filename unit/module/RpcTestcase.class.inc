<?php
require_once 'unit/SyndNodeTestCase.class.inc';

/**
 * @package		synd.unit
 */
abstract class RpcTestcase extends SyndNodeTestCase {
	function setUp() {
		require_once 'core/lib/rpc/RpcTransport.class.inc';
		require_once 'core/lib/rpc/RpcMessage.class.inc';
	}
	
	abstract function getActivator();
	
	function testEncoding() {
		$node = SyndNodeLib::factory('unit_test');
		$node->save();
		$node->flush();
		
		$remote = Activator::getInstance($this->getActivator()->getEndpoint($node));
		
		// Integer
		$expected = 123;
		$actual = $remote->echo($expected);
		$this->assertEquals($expected, $actual);

		// Float
		$expected = 123.123;
		$actual = $remote->echo($expected);
		$this->assertEquals($expected, $actual);

		// String
		$expected = 'abc';
		$actual = $remote->echo($expected);
		$this->assertEquals($expected, $actual);

		$expected = 'abc <foo> def';
		$actual = $remote->echo($expected);
		$this->assertEquals($expected, $actual);

		$expected = str_repeat('abcd', 1024*256);
		$actual = $remote->echo($expected);
		$this->assertEquals($expected, $actual);

		// Test array
		$expected = array('a','b');
		$actual = $remote->echo($expected);
		$this->assertEquals($expected, $actual);

		// Test hashmap
		$expected = array('a'=>'a','b'=>'b');
		$actual = $remote->echo($expected);
		$this->assertEquals($expected, $actual);
	}

	function testRemoting() {
		global $synd_maindb;
		$expected = 'Test123';
		
		$node = SyndNodeLib::factory('unit_test');
		$node->data['INFO_HEAD'] = $expected;
		$node->_string = $expected;
		$node->save();
		$node->flush();
		
		$remote = Activator::getInstance($this->getActivator()->getEndpoint($node));
			
		$this->assertEquals($expected, $node->toString());
		$this->assertEquals($expected, $remote->toString());
		$this->assertEquals($expected, $remote->_string);

		$self = $remote->getSelf();
		$this->assertEquals($expected, $self->toString());
	}

	function testActivation() {
		global $synd_maindb;
		$expected = 'Test123';
		
		$node1 = SyndNodeLib::factory('unit_test');
		$node1->data['INFO_HEAD'] = $expected;
		$node1->save();
		$node1->flush();
		
		$node2 = SyndNodeLib::factory('unit_test');
		$node2->save();
		$node2->flush();

		$remote = Activator::getInstance($this->getActivator()->getEndpoint($node2), true, $this->getActivator());
			
		$actual = $remote->proxy($node1);
		$this->assertEquals($expected, $actual);
	}
}
