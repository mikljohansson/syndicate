<?php
require_once 'RpcTestcase.class.inc';

class _modules_XMLRPC extends RpcTestcase {
	function getActivator() {
		return new RpcModuleActivator(Module::getInstance('rpc'), 'xmlrpc');
	}

	function testEncodeObject() {
		$node = SyndNodeLib::factory('unit_test');
		$node->save();
		$node->flush();

		$remote = Activator::getInstance($this->getActivator()->getEndpoint($node));

		$expected = array('a'=>'a','b'=>'b');
		$value = new stdClass();
		$value->a = 'a';
		$value->b = 'b';
		$actual = $remote->echo($value);
		$this->assertEquals($expected, $actual);
	}
}
