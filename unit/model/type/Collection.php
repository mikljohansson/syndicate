<?php

class _model_Type_Collection extends PHPUnit2_Framework_TestCase {
	function testInstance() {
		$collection = SyndType::factory('collection');
		$id = $collection->id();
		
		$actual = SyndLib::getInstance($id);
		$this->assertSame($collection, $actual);

		$collection->delete();
		$actual = SyndLib::getInstance($id);
		$this->assertNull($actual);
	}
}
