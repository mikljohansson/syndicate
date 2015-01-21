<?php
require_once 'unit/SyndNodeTestCase.class.inc';

class _model_Node_Task extends SyndNodeTestCase {
	function testSerialization() {
		$task = SyndNodeLib::factory('task');
		$expected = 'Test';

		$task->setDescription($expected);
		$actual = $task->getDescription();
		$this->assertEquals($expected, $actual);

		$task2 = unserialize(serialize($task));
		
		$actual = $task2->getDescription();
		$this->assertEquals($expected, $actual);
	}
}
