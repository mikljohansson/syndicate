<?php
require_once 'unit/SyndNodeTestCase.class.inc';

class _model_Node_Questionnaire extends SyndNodeTestCase {
	function testQuestions() {
		$page = SyndNodeLib::factory('questionnaire');
		$page->save();
		
		$question = $page->_storage->factory('multiple_choice');
		$question->setParent($page);
		$question->save();
		
		$page->_storage->flush();
		
		$children = $page->getChildren();
		$this->assertTrue(isset($children[$question->nodeId]));
		
		$question->delete();
		$page->_storage->flush();

		$children = $page->getChildren();
		$this->assertFalse(isset($children[$question->nodeId]));

		$page->delete();
	}
}
