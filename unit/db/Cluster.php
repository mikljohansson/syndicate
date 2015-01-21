<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _db_Cluster extends PHPUnit2_Framework_TestCase {
	function testConnect() {
		global $synd_maindb;
		$dsn = 'cluster://'.$synd_maindb->getDSN().';'.$synd_maindb->getDSN().','.$synd_maindb->getDSN();
		$connection = DatabaseManager::getConnection($dsn);
		
		$dsn = 'cluster:balanced://'.$synd_maindb->getDSN().';'.$synd_maindb->getDSN().','.$synd_maindb->getDSN();
		$connection = DatabaseManager::getConnection($dsn);

		$dsn = 'cluster:hotstandby://'.$synd_maindb->getDSN().';'.$synd_maindb->getDSN().','.$synd_maindb->getDSN();
		$connection = DatabaseManager::getConnection($dsn);
	}
}
