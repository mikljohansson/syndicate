<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'RpcTestcase.class.inc';

class _modules_SOAP extends RpcTestcase {
	function getActivator() {
		return new RpcModuleActivator(Module::getInstance('rpc'), 'soap');
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
	
	function testNamespace() {
		require_once 'core/lib/rpc/RpcTransport.class.inc';
		$transport = Activator::getTransport('soap://Assembly/Namespace.Interface;http://www.example.com/synd/soap/issue.123/', false);
		$formatter = $transport->getFormatter();
		$this->assertEquals("http://schemas.microsoft.com/clr/nsassem/Namespace.Interface/Assembly#{method}", $formatter->_action);
	}
}
