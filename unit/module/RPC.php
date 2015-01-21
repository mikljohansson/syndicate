<?php
require_once 'RpcTestcase.class.inc';

class _modules_RPC extends RpcTestcase {
	function setUp() {
		require_once 'core/lib/rpc/formatter/php.class.inc';
	}
	
	function getActivator() {
		return new RpcModuleActivator(Module::getInstance('rpc'), 'php');
	}
	
	function testAdvancedEncoding() {
		$node = SyndNodeLib::factory('unit_test');
		$node->save();
		$node->flush();
		
		$remote = Activator::getInstance($this->getActivator()->getEndpoint($node));
		
		// Test integer indexes
		$expected = array('123'=>'a','456'=>'b');
		$actual = $remote->echo($expected);
		$this->assertEquals($expected, $actual);

		// Test mixed hashmap and integer indexes
		$expected = array('a'=>'a',5=>'c');
		$actual = $remote->echo($expected);
		$this->assertEquals($expected, $actual);
	}

	function testUnserializeObject() {
		$formatter = synd_formatter_php::factory('php');
		
		try {
			$actual = true;
			$formatter->getNativeValue(new HttpMessage(serialize(new RpcUnittestClass())));
		}
		catch (RemotingException $e) {$actual = false;}
		$this->assertFalse($actual);
		
		try {
			$actual = true;
			$formatter->getNativeValue(new HttpMessage(strtoupper(serialize(new RpcUnittestClass()))));
		}
		catch (RemotingException $e) {$actual = false;}
		$this->assertFalse($actual);

		$actual = $formatter->getNativeValue(new HttpMessage(serialize(serialize(new RpcUnittestClass()))));
		$this->assertType('string', $actual);

		$actual = $formatter->getNativeValue(new HttpMessage(serialize(array(
			serialize(new RpcUnittestClass()),
			serialize(new RpcUnittestClass()),))));
		$this->assertType('array', $actual);

		$actual = $formatter->getNativeValue(new HttpMessage(serialize(
			serialize('abc').serialize(new RpcUnittestClass()).
			serialize(new RpcUnittestClass()).serialize('abc'))));
		$this->assertType('string', $actual);

		$actual = $formatter->getNativeValue(new HttpMessage(serialize('O:')));
		$this->assertType('string', $actual);
		$actual = $formatter->getNativeValue(new HttpMessage(serialize(' O:')));
		$this->assertType('string', $actual);
		$actual = $formatter->getNativeValue(new HttpMessage(serialize('O: ')));
		$this->assertType('string', $actual);
		$actual = $formatter->getNativeValue(new HttpMessage(serialize(' O: ')));
		$this->assertType('string', $actual);
	}
}

/**
 * @access	private
 */
class RpcUnittestClass {
	var $_test = true;
}
