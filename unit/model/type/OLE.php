<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_Type_OLE extends PHPUnit2_Framework_TestCase {
	function testLoadInstance() {
		$expected = SyndType::factory('cut');
		$actual = SyndLib::getInstance($expected->id());
		$this->assertEquals($expected, $actual);
		
		$expected->delete();
		$actual = SyndLib::getInstance($expected->id());
		
		$this->assertNull($actual);
	}
}