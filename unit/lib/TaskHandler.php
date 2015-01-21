<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/lib/SyndTaskHandler.class.inc';

class _lib_TaskHandler extends PHPUnit2_Framework_TestCase {
	var $_handler = null;
	
	function setUp() {
		$this->_handler = SyndTaskHandler::factory('unit_test');
	}
	
	function testTaskMultiple() {
		global $synd_config;
		$file1 = $synd_config['dirs']['cache'].'unit_test_task1';
		$file2 = $synd_config['dirs']['cache'].'unit_test_task2';
		$file3 = $synd_config['dirs']['cache'].'unit_test_task3';
		
		@unlink($file1);
		@unlink($file2);
		@unlink($file3);

		$this->assertFalse(file_exists($file1));
		$this->assertFalse(file_exists($file2));

		$this->assertFalse($this->_handler->isScheduled('unit_test_task1'));
		$this->assertFalse($this->_handler->isScheduled('unit_test_task2'));
		$this->assertFalse($this->_handler->isScheduled('unit_test_task3'));

		$this->_handler->appendTask(SyndType::factory('unit_test_task', $file1), 'unit_test_task1');
		$this->_handler->appendTask(SyndType::factory('unit_test_task', $file2), 'unit_test_task2');
		$this->_handler->appendTask(SyndType::factory('unit_test_task', $file3), 'unit_test_task3');

		$this->assertTrue($this->_handler->isScheduled('unit_test_task1'));
		$this->assertTrue($this->_handler->isScheduled('unit_test_task2'));
		$this->assertTrue($this->_handler->isScheduled('unit_test_task3'));
		$this->assertTrue($this->_handler->isRunning());

		for ($t=time(); time()<$t+5 && (!file_exists($file1) || !file_exists($file2) || !file_exists($file3)); usleep(200000));
		
		$this->assertTrue(file_exists($file1));
		$this->assertTrue(file_exists($file2));
		$this->assertTrue(file_exists($file3));
		
		$this->assertEquals('Test', file_get_contents($file1));
		$this->assertEquals('Test', file_get_contents($file2));
		$this->assertEquals('Test', file_get_contents($file3));
		
		@unlink($file1);
		@unlink($file2);
		@unlink($file3);
	}
}
