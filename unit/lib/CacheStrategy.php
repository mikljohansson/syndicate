<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _lib_CacheStrategy extends PHPUnit2_Framework_TestCase {
	function setUp() {
		parent::setUp();
		require_once 'core/lib/CacheStrategy.class.inc';
	}
	
	function _testCache($strategy) {
		// Test set
		$this->assertTrue($strategy->set('_unit_test', 'bar'));
		
		// Test get
		$this->assertEquals('bar', $strategy->get('_unit_test'));
		
		// Test delete
		$this->assertTrue($strategy->delete('_unit_test'));
		$this->assertFalse($strategy->get('_unit_test'));
		
		// Test multiple get
		$expected = array(
			'_unit_test_multi1' => uniqid(''),
			'_unit_test_multi2' => uniqid(''),
			);
		
		foreach ($expected as $key => $value)
			$this->assertTrue($strategy->set($key, $value));
		
		$keys = array_keys($expected);
		$keys[] = '_unit_test_multi3';
		
		$actual = (array)$strategy->get($keys);
		ksort($actual);
		$this->assertEquals($expected, $actual);

		foreach ($expected as $key => $value)
			$this->assertTrue($strategy->delete($key));
		
		// Test add
		$this->assertTrue($strategy->add('_unit_test', 'bar'));
		$this->assertFalse($strategy->add('_unit_test', 'bar'));
		
		// Test replace
		$this->assertTrue($strategy->replace('_unit_test', 'foo'));
		$this->assertTrue($strategy->delete('_unit_test'));
		$this->assertFalse($strategy->replace('_unit_test', 'foo'));
		$this->assertFalse($strategy->get('_unit_test'));
		
		// Test increment
		$this->assertFalse($strategy->increment('_unit_test'));
		$this->assertFalse($strategy->get('_unit_test'));
		
		$this->assertTrue($strategy->set('_unit_test', 0));
		$this->assertEquals(1, $strategy->increment('_unit_test'));
		$this->assertEquals(1, $strategy->get('_unit_test'));
		
		$this->assertEquals(3, $strategy->increment('_unit_test', 2));
		$this->assertEquals(3, $strategy->get('_unit_test'));
		
		// Test decrement
		$this->assertEquals(1, $strategy->decrement('_unit_test', 2));
		$this->assertEquals(1, $strategy->get('_unit_test'));

		$this->assertEquals(0, $strategy->decrement('_unit_test'));
		$this->assertEquals(0, $strategy->get('_unit_test'));
		
		$this->assertTrue($strategy->delete('_unit_test'));
		$this->assertFalse($strategy->decrement('_unit_test'));
	}
	
	function testGenericCache()	{
		$this->_testCache(CacheStrategyManager::factory());
	}
	
	function testFilesystemCacheStrategy() {
		global $synd_config;
		$path = isset($synd_config['dirs']['cache']) ? $synd_config['dirs']['cache'].'fs/' : 
			(isset($_SERVER['SERVER_NAME']) ? "/tmp/{$_SERVER['SERVER_NAME']}/fs/" : '/tmp/local/fs/');
		$this->_testCache(new FilesystemCacheStrategy($path));
	}

	function testMemcacheCacheStrategy() {
		global $synd_config;
		if (!function_exists('memcache_set') || empty($synd_config['cache']['memcached']))
			$this->markTestSkipped();
		$this->_testCache(new MemcachedStrategy($synd_config['cache']['memcached']));
	}

	function testApcCacheStrategy() {
		if (!function_exists('apc_store'))
			$this->markTestSkipped();
		$this->_testCache(new ApcCacheStrategy());
	}

	function testXCacheCacheStrategy() {
		if (!function_exists('xcache_set'))
			$this->markTestSkipped();
		$this->_testCache(new XCacheStrategy());
	}
	
	function testNamespace() {
		$strategy = CacheStrategyManager::factory();
		$this->_testCache($strategy);
		
		$this->assertTrue($strategy->set('_unit_test/test', 1));
		$this->assertEquals(1, $strategy->get('_unit_test/test'));
		
		$strategy->clear('_unit_test');
		$this->assertEquals(false, $strategy->get('_unit_test/test'));


		$this->assertTrue($strategy->set('_unit_test/test', 1));
		$this->assertTrue($strategy->set('_unit_test/ns2/test', 1));
		$this->assertEquals(1, $strategy->get('_unit_test/test'));
		$this->assertEquals(1, $strategy->get('_unit_test/ns2/test'));
		
		$strategy->clear('_unit_test');
		$this->assertEquals(false, $strategy->get('_unit_test/test'));
		$this->assertEquals(false, $strategy->get('_unit_test/ns2/test'));


		$this->assertTrue($strategy->set('_unit_test/test', 1));
		$this->assertTrue($strategy->set('_unit_test/ns2/test', 1));
		$this->assertEquals(1, $strategy->get('_unit_test/test'));
		$this->assertEquals(1, $strategy->get('_unit_test/ns2/test'));
		
		$strategy->clear('_unit_test/ns2');
		$this->assertEquals(1, $strategy->get('_unit_test/test'));
		$this->assertEquals(false, $strategy->get('_unit_test/ns2/test'));
	}
}
