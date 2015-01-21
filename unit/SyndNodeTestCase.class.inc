<?php
require_once 'PHPUnit2/Framework/TestCase.php';

/**
 * Node testcase superclass.
 *
 * Provides general setUp() functionality for the framework so
 * that individual test cases don't have do extensive cleaning up
 * on their own.
 *
 * @package	synd.unit
 */
abstract class SyndNodeTestCase extends PHPUnit2_Framework_TestCase {
	function setUp() {
		global $synd_maindb;
		$synd_maindb->rollback();
		$this->assertFalse(SyndLib::isError($synd_maindb->query('DELETE FROM synd_unit_test')));
	}
	
	function getDatabaseStorage() {
		global $synd_maindb;
		require_once 'core/model/storage/DomainStorage.class.inc';
		return DomainStorageManager::getStorage(array('database:'.$synd_maindb->getDSN()));
	}
}
