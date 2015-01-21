<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/lib/SessionHandler.class.inc';
require_once 'core/lib/CacheStrategy.class.inc';

class _lib_SessionHandler extends PHPUnit2_Framework_TestCase {
	function testDatabase() {
		global $synd_maindb;
		$handler = new	DatabaseSessionHandler($synd_maindb);
		$this->assertTrue($handler->write('_unit_test', 'Test'));
		
		$actual = $handler->read('_unit_test');
		$this->assertEquals('Test', $actual);
		
		$this->assertTrue($handler->destroy('_unit_test'));
		
		$actual = $handler->read('_unit_test');
		$this->assertEquals('', $actual);
	}
	
	function testCached() {
		global $synd_maindb;
		$strategy = CacheStrategyManager::factory();
		$handler = new CachedSessionHandler(new DatabaseSessionHandler($synd_maindb), $strategy);
		$this->assertTrue($handler->write('_unit_test', 'Test'));

		$actual = $handler->read('_unit_test');
		$this->assertEquals('Test', $actual);
		
		$actual = $strategy->get('session._unit_test');
		$this->assertEquals('Test', $actual);
		
		$this->assertTrue($handler->destroy('_unit_test'));
		
		$actual = $handler->read('_unit_test');
		$this->assertEquals('', $actual);

		$actual = $strategy->get('session._unit_test');
		$this->assertFalse($actual);
	}
	
	function testBlocking() {
		$canary = new UnittestSessionHandler();
		$handler = new BlockingSessionHandler($canary);

		$this->assertTrue($handler->write('_unit_test', 'Test'));
		$this->assertEquals(1, $canary->_writes);

		$this->assertTrue($handler->write('_unit_test', 'Test'));
		$this->assertEquals(1, $canary->_writes);

		$this->assertTrue($handler->write('_unit_test', 'Test2'));
		$this->assertEquals(2, $canary->_writes);

		// Test blocking empty new sessions writes
		$canary = new UnittestSessionHandler();
		$handler = new BlockingSessionHandler($canary);

		$this->assertTrue($handler->write('_unit_test', ''));
		$this->assertEquals(0, $canary->_writes);

		$this->assertTrue($handler->write('_unit_test', 'Test'));
		$this->assertEquals(1, $canary->_writes);

		$this->assertTrue($handler->write('_unit_test', ''));
		$this->assertEquals(2, $canary->_writes);

		$this->assertTrue($handler->write('_unit_test', 'Test2'));
		$this->assertEquals(3, $canary->_writes);

		// Test letting write empty through with old session
		$handler = new BlockingSessionHandler($canary);
		$this->assertEquals('Test2', $handler->read('_unit_test'));
		$this->assertTrue($handler->write('_unit_test', ''));
		$this->assertEquals(4, $canary->_writes);
	}
}

class UnittestSessionHandler extends SessionHandler {
	var $_writes = 0;
	var $_values = array();

	function open($path, $id) {
		return true;
	}

	function close() {
		return true;
	}
	
	function write($id, $content) {
		$this->_values[$id] = $content;
		$this->_writes++;
		return true;
	}
	
	function read($id) {
		return isset($this->_values[$id]) ? $this->_values[$id] : null;
	}
	
	function destroy($id) {
		return true;
	}
	
	function gc($ttl) {
		return true;
	}
}
