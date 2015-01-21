<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _db_SqlQuery extends PHPUnit2_Framework_TestCase {
	function testTables() {
		global $synd_maindb;
		$query = $synd_maindb->createQuery();
		$query->join('test1');
		$actual = $query->getTables();
		$this->assertEquals(array('test1'), $actual);

		$query->join('test2');
		$actual = $query->getTables();
		$this->assertEquals(array('test1','test2'), $actual);
	}
}
