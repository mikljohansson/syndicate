<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _modules_Storage extends PHPUnit2_Framework_TestCase {
	var $_path = null;
	var $_uri = null;
	
	function setUp() {
		global $synd_config;
		require_once 'core/lib/rpc/RpcTransport.class.inc';

		$this->_path = $synd_config['dirs']['var']['path'].'_unit_test/';
		$this->_uri = tpl_request_host().$synd_config['dirs']['var']['uri'].'_unit_test/';
		
		SyndLib::createDirectory($this->_path);
		$this->assertTrue(file_exists($this->_path));
	}

	function tearDown() {
		SyndLib::unlink($this->_path, true);
		$this->assertFalse(file_exists($this->_path));
	}

	function testDevice() {
		$device = Activator::getInstance('local;device');
		$this->assertNotNull($device);
		if (null == $device) return;
		
		$file1 = $this->_path.'test1.txt';
		$file2 = $this->_path.'test2.txt';
		$uri = $this->_uri.'test1.txt';
		$data = 'Test';
		
		// Test set() method
		SyndLib::file_put_contents($file1, $data);
		$this->assertTrue(file_exists($file1));
		
		$actual = $device->set('_unit_test', $file1, $uri, md5_file($file1), strlen($data));
		$this->assertTrue($actual);
		
		$uri = $device->get('_unit_test', $file1, strlen($data));
		$this->assertEquals($data, @file_get_contents($uri));

		$actual = @$device->set('_unit_test', $file1, $uri, 'not-a-checksum', strlen($data));
		$this->assertFalse($actual);

		// Test set empty file
		SyndLib::file_put_contents($file1, '');
		$actual = $device->set('_unit_test', $file1, $uri, md5(''), 0);
		$this->assertTrue($actual);

		$uri = $device->get('_unit_test', $file1, 0);
		$this->assertNotNull($uri);
		$this->assertEquals('', file_get_contents($uri));
		
		// Test put() method
		$actual = $device->put('_unit_test', $file2, $data, md5($data));
		$this->assertTrue($actual);

		$uri = $device->get('_unit_test', $file2);
		$this->assertNotNull($uri);
		if (null != $uri) {
			$this->assertEquals($data, file_get_contents($uri));
		}
		
		$actual = @$device->put('_unit_test', $file2, $data, 'not-a-checksum');
		$this->assertFalse($actual);
		
		// Test checksumming
		$expected = array($file1 => md5_file($file1));
		$actual = $device->checksum('_unit_test', array($file1, md5(uniqid(''))));
		$this->assertEquals($expected, $actual);
		
		$actual = $device->delete('_unit_test', $file1);
		$this->assertTrue($actual);
		$actual = $device->delete('_unit_test', $file2);
		$this->assertTrue($actual);
	}
	
	function testStorage() {
		$file = $this->_path.'_unit_test_'.md5(uniqid('')).'.txt';
		$data = 'Test';
		
		SyndLib::file_put_contents($file, $data);
		$this->assertTrue(file_exists($file));

		$ts = time();
		$actual = SyndLib::runHook('lob_storage_set', $file, $file, 1);
		$this->assertTrue($actual);
		
		// Test 'lob_storage_set'
		$uri = SyndLib::runHook('lob_storage_uri', $file);
		$actual = file_get_contents($uri);
		$this->assertEquals($data, $actual);
		
		// Test 'lob_storage_copy'
		$file2 = $this->_path.'_unit_test_'.md5(uniqid('')).'.txt';
		$actual = SyndLib::runHook('lob_storage_copy', $file, $file2);
		$this->assertTrue($actual);
		
		$uri2 = SyndLib::runHook('lob_storage_uri', $file2, true, true);
		$actual = file_get_contents($uri2);
		$this->assertEquals($data, $actual);
		
		// Test 'lob_storage_put'
		$file3 = $this->_path.'_unit_test_'.md5(uniqid('')).'.txt';
		$actual = SyndLib::runHook('lob_storage_copy', $file, $file3);
		$this->assertTrue($actual);

		// Test inode information
		$actual = SyndLib::runHook('lob_storage_stat', $file, $file);
		$expected = array(
			'size' => (float)strlen($data),
			'mtime' => (int)$ts,
			'ctime' => (int)$ts,);
		$this->assertEquals($expected, SyndLib::array_kintersect($expected, $actual));

		// Test delete
		$actual = SyndLib::runHook('lob_storage_delete', $file);
		$this->assertTrue($actual);
		$actual = SyndLib::runHook('lob_storage_delete', $file2);
		$this->assertTrue($actual);
		
		// Test delete recursive
		$actual = SyndLib::runHook('lob_storage_delete', substr($file3,0,-4), true);
		$this->assertTrue($actual);
		
		$uri = SyndLib::runHook('lob_storage_uri', $file);
		$actual = @file_get_contents($uri);
		$this->assertFalse($actual);

		$uri = SyndLib::runHook('lob_storage_uri', $file2);
		$actual = @file_get_contents($uri);
		$this->assertFalse($actual);
		
		$uri = SyndLib::runHook('lob_storage_uri', $file3);
		$actual = @file_get_contents($uri);
		$this->assertFalse($actual);

		sleep(5);
	}
	
	function testNormalize() {
		$storage = Module::getInstance('storage');
		$tracker = $storage->getTracker();
		
		$file1 = $this->_path.'_unit_test_'.md5(uniqid('')).'.txt';
		$file2 = $this->_path.'_unit_test_'.md5(uniqid('')).'.txt';
		$file3 = $this->_path.'_unit_test_'.md5(uniqid('')).'.txt';

		$data1 = "Test1";
		$data2 = str_repeat('abc', $storage->_inline - 1);
		$data3 = str_repeat('abc', $storage->_inline + 1);
		
		SyndLib::file_put_contents($file1, $data1);
		$this->assertTrue(file_exists($file1));
		SyndLib::file_put_contents($file2, $data2);
		$this->assertTrue(file_exists($file2));
		SyndLib::file_put_contents($file3, $data3);
		$this->assertTrue(file_exists($file3));

		$actual = SyndLib::runHook('lob_storage_set', $file1, $file1, 2);
		$this->assertTrue($actual);
		$actual = SyndLib::runHook('lob_storage_set', $file2, $file2, 2);
		$this->assertTrue($actual);
		
		// Test asyncronous replication
		sleep(15);
		
		$uris1 = $tracker->getLocations($storage->getNamespace(), trim($file1, '/'));
		$this->assertEquals(2, count($uris1));
		foreach ($uris1 as $uri) {
			$actual = file_get_contents($uri);
			$this->assertEquals($data1, $actual);
		}

		$uris2 = $tracker->getLocations($storage->getNamespace(), trim($file2, '/'));
		$this->assertEquals(2, count($uris2));
		foreach ($uris2 as $uri) {
			$actual = file_get_contents($uri);
			$this->assertEquals($data2, $actual);
		}

		// Rewrite file1 with only one replica	
		$actual = SyndLib::runHook('lob_storage_set', $file1, $file1, 1);
		$this->assertTrue($actual);

		// Rewrite file2 with different checksum
		$actual = SyndLib::runHook('lob_storage_set', $file2, $file3, 2);
		$this->assertTrue($actual);
		
		// Execute normalize procedure
		$tracker->_callback_garbage_collect_replicas();		
		$tracker->_callback_normalize();		
		
		// Test that one of file1 uris have been deleted
		$deleted = 0;
		foreach ($uris1 as $uri) {
			if (false == ($actual = @file_get_contents($uri)))
				$deleted++;
		}
		$this->assertEquals(1, $deleted);

		// Test that one of file2 uris have been deleted or both copies have been updated
		$count = 0;
		foreach ($uris2 as $uri) {
			if (false == ($actual = @file_get_contents($uri)) || $data3 == $actual)
				$count++;
		}
		$this->assertEquals(2, $count);

		// Remove test files
		$actual = SyndLib::runHook('lob_storage_delete', $file1);
		$this->assertTrue($actual);
		$actual = SyndLib::runHook('lob_storage_delete', $file2);
		$this->assertTrue($actual);
	}

	function testTracker() {
		global $synd_maindb;
		$sql = "SELECT COUNT(*) FROM synd_storage_lob l";
		if ($synd_maindb->getOne($sql) > 25) {
			$this->assertTrue(false, 'This test is not to be run against a production database');
			return;
		}
		
		$storage = Module::getInstance('storage');
		$tracker = $storage->getTracker();
		
		$file1 = $this->_path.'_unit_test_'.md5(uniqid('')).'-1.txt';
		$file2 = $this->_path.'_unit_test_'.md5(uniqid('')).'-2.txt';
		$file3 = $this->_path.'_unit_test_'.md5(uniqid('')).'-3.txt';
		$file4 = $this->_path.'_unit_test_'.md5(uniqid('')).'-4.txt';
		
		$data = "Test\0\1\2\3abc";
		$data3 = str_repeat('a', $storage->_inline - 1);
		$data4 = str_repeat('a', $storage->_inline + 1);
		
		SyndLib::file_put_contents($file1, $data);
		$this->assertTrue(file_exists($file1));
		SyndLib::file_put_contents($file2, $data);
		SyndLib::file_put_contents($file3, $data3);
		SyndLib::file_put_contents($file4, $data4);

		$actual = SyndLib::runHook('lob_storage_set', $file1, $file1, 2);
		$this->assertTrue($actual);
		$actual = SyndLib::runHook('lob_storage_set', $file2, $file2, 255);
		$this->assertTrue($actual);
		$actual = SyndLib::runHook('lob_storage_set', $file3, $file3, 2);
		$this->assertTrue($actual);
		$actual = SyndLib::runHook('lob_storage_set', $file4, $file4, 2);
		$this->assertTrue($actual);
		
		// Test syncronous storage to first replica
		$uri = SyndLib::runHook('lob_storage_uri', $file1);
		$actual = file_get_contents($uri);
		$this->assertEquals($data, $actual);
		
		// Test asyncronous replication
		sleep(15);
		
		// Test all replicas
		$uris = $tracker->getLocations($storage->getNamespace(), trim($file1, '/'));
		$this->assertEquals(2, count($uris));
		foreach ($uris as $uri) {
			$actual = file_get_contents($uri);
			$this->assertEquals($data, $actual);
		}

		$uris2 = $tracker->getLocations($storage->getNamespace(), trim($file2, '/'));
		$uris3 = $tracker->getLocations($storage->getNamespace(), trim($file3, '/'));
		$uris4 = $tracker->getLocations($storage->getNamespace(), trim($file4, '/'));
		$this->assertFalse(empty($uris2));
		$this->assertEquals(2, count($uris3));
		$this->assertEquals(2, count($uris4));

		// Test syncronous delete
		$actual = SyndLib::runHook('lob_storage_delete', $file1);
		$this->assertTrue($actual);
		$uri = SyndLib::runHook('lob_storage_uri', $file1);
		$actual = @file_get_contents($uri);
		$this->assertFalse($actual);
		
		// Test asyncronous garbage collection on all replicas
		sleep(15);

		foreach ($uris as $uri) {
			$actual = @file_get_contents($uri);
			$this->assertEquals(false, $actual);
		}
		
		// Corrupt all file2 replicas so all devices will be resynced
		$devices2 = $tracker->getDevices($storage->getNamespace(), trim($file2, '/'));
		$this->assertFalse(empty($devices2));
		foreach (array_keys($devices2) as $key) {
			$corrupt = 'Some corrupt content';
			$actual = $devices2[$key]->put($storage->getNamespace(), trim($file2, '/'), $corrupt, md5($corrupt));
			$this->assertTrue($actual);
		}
		
		// Delete one of the file3 replicas
		$devices3 = $tracker->getDevices($storage->getNamespace(), trim($file3, '/'));
		$this->assertEquals(2, count($devices3));
		foreach (array_keys($devices3) as $key) {
			$actual = $devices3[$key]->delete($storage->getNamespace(), trim($file3, '/'));
			$this->assertTrue($actual);
			break;
		}
		
		// Corrupt one of the file4 replicas
		$devices4 = $tracker->getDevices($storage->getNamespace(), trim($file4, '/'));
		$this->assertEquals(2, count($devices4));
		foreach (array_keys($devices4) as $key) {
			$corrupt = 'Some corrupt content';
			$actual = $devices4[$key]->put($storage->getNamespace(), trim($file4, '/'), $corrupt, md5($corrupt));
			$this->assertTrue($actual);
			break;
		}
		
		// Request corrupt file2 in order to trigger self-healing routine
		$uri = $tracker->get($storage->getNamespace(), trim($file2, '/'));
		$this->assertFalse($uri);
		
		sleep(5);
		
		// Trigger replication since all devices were offline the first time
		$uri = $tracker->_callback_replicate();
		$this->assertFalse($uri);
		
		sleep(5);

		// Check that we again have 2 replicas of file3
		$uris = $tracker->getLocations($storage->getNamespace(), trim($file3, '/'));
		$this->assertEquals(2, count($uris));
		foreach ($uris as $uri) {
			$actual = file_get_contents($uri);
			$this->assertEquals($data3, $actual);
		}
		
		// Check that we have 2 consistent replicas of file4
		$uris = $tracker->getLocations($storage->getNamespace(), trim($file4, '/'));
		$this->assertEquals(2, count($uris));
		foreach ($uris as $uri) {
			$actual = file_get_contents($uri);
			$this->assertEquals($data4, $actual);
		}
		
		// Remove test files
		$actual = SyndLib::runHook('lob_storage_delete', $file2);
		$this->assertTrue($actual);
		$actual = SyndLib::runHook('lob_storage_delete', $file3);
		$this->assertTrue($actual);
		$actual = SyndLib::runHook('lob_storage_delete', $file4);
		$this->assertTrue($actual);
	}
}
