<?php
require_once 'unit/SyndNodeTestCase.class.inc';

class _model_Node_Repair extends SyndNodeTestCase {
	function testPrinter() {
		$repair = SyndNodeLib::factory('repair');
		$this->assertNotNull($repair->getPrinter());
	}
}