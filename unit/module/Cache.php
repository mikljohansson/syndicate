<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/lib/TemplateLib.inc';

class _modules_Cache extends PHPUnit2_Framework_TestCase {
	function setUp() {
		Module::getInstance('cache');
		SyndLib::runHook('cache_del', '_unit_test');
	}

	function tearDown() {
		SyndLib::runHook('cache_del', '_unit_test');
	}
	
	function testCache() {
		$expected = 'Test';
		SyndLib::runHook('cache_set', '_unit_test', $expected);
		
		$actual = SyndLib::runHook('cache_get', '_unit_test');
		$this->assertEquals($expected, $actual);

		SyndLib::runHook('cache_del', '_unit_test');
		$actual = SyndLib::runHook('cache_get', '_unit_test');
		$this->assertNull($actual);
	}
	
	function testOutputCache() {
		$this->assertFalse(tpl_cache_display('_unit_test'));
		
		ob_start();
		
		tpl_cache_enter('_unit_test');
		print 'foo';
		tpl_cache_leave('_unit_test');
		
		$output = ob_get_clean();
		$this->assertEquals('foo', $output);
		
		ob_start();
		$this->assertTrue(tpl_cache_display('_unit_test'));
		$output = ob_get_clean();
		$this->assertEquals('foo', $output);
		
		tpl_cache_delete('_unit_test');
		$this->assertFalse(tpl_cache_display('_unit_test'));
	}
}

