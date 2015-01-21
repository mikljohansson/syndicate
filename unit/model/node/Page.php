<?php
require_once 'unit/SyndNodeTestCase.class.inc';

class _model_Node_Page extends SyndNodeTestCase {
	function testContent() {
		$page = SyndNodeLib::factory('page');
		$expected = $page->getContent();
		$actual = SyndLib::getInstance($expected->id());
		
		$this->assertNotNull($actual);
		$this->assertEquals($expected, $actual);
	}
}
