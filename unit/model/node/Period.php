<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_Node_Period extends PHPUnit2_Framework_TestCase {
	function testCreation() {
		$project = SyndNodeLib::factory('project');
		$project->setTitle('_unit_test: This project is safe to delete');
		
		$period = $project->appendChild(SyndNodeLib::factory('period'));
		
		$period->delete();
		$project->delete();
	}

}