<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _lib_Date extends PHPUnit2_Framework_TestCase {
	function setUp() {
		require_once 'core/lib/SyndDate.class.inc';
	}
	
	function testChecktime() {
		$this->assertTrue(SyndDate::checktime('00:00'));
		$this->assertTrue(SyndDate::checktime('01:00'));
		$this->assertTrue(SyndDate::checktime('24:00'));
		$this->assertFalse(SyndDate::checktime('24:30'));
		$this->assertFalse(SyndDate::checktime('123'));
	}
}