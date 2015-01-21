<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/model/DomainObject.class.inc';

class _model_Workflow extends PHPUnit2_Framework_TestCase {
	function testMethod() {
		$subject = new _model_Workflow_Subject();
		$expected = 'Test';
		
		$workflow = SyndNodeLib::factory('workflow');
		$workflow->handleEvent(new synd_event_create($subject));

		$activity = new synd_workflow_method('_model_Workflow_Subject', 'callback1', array($expected), 'Test');
		$activity->setOption($expected);
		
		$workflow->setActivity($activity);
		$workflow->handleEvent(new synd_event_create($subject));
		
		$this->assertEquals($expected, $subject->_param1);
	}
	
	function testSequence() {
		$subject = new _model_Workflow_Subject();
		$expected1 = 'Test1';
		$expected2 = 'Test2';
		
		$activity1 = new synd_workflow_method('_model_Workflow_Subject', 'callback1', array($expected1), 'Test');
		$activity1->setOption($expected1);
		$activity2 = new synd_workflow_method('_model_Workflow_Subject', 'callback2', array($expected2), 'Test');
		$activity2->setOption($expected2);
		
		$sequence = new synd_workflow_sequence();
		$sequence->addActivity($activity1);
		$sequence->addActivity($activity2);
		
		$workflow = SyndNodeLib::factory('workflow');
		$workflow->setActivity($sequence);
		$workflow->handleEvent(new synd_event_create($subject));
		
		$this->assertEquals($expected1, $subject->_param1);
		$this->assertEquals($expected2, $subject->_param2);
	}
	
	function testEventing() {
		Module::getInstance('issue')->loadPlugin('workflow');
		
		$subject = new _model_Workflow_Subject();
		$context = SyndNodeLib::factory('project');
		$context->data['INFO_HEAD'] = '_unit_test: This project is safe to delete';		
		$context->save();
		
		$expected = 'Test';
		
		$workflow = $context->_storage->factory('workflow');
		$workflow->data['INFO_HEAD'] = '_unit_test: This workflow is safe to delete';		
		$workflow->setParent($context);

		$activity = new synd_workflow_method('_model_Workflow_Subject', 'callback1', array($expected), 'Test');
		$activity->setOption($expected);

		$workflow->setActivity($activity);
		$workflow->attachEvent('AbstractDomainEvent');
		$workflow->save();
		
		$context->_storage->flush();
		
		SyndLib::runHook('event', $context, new synd_event_create($subject));
		$this->assertEquals($expected, $subject->_param1);
		
		$context->delete();
		$context->_storage->flush();
	}
}

class _model_Workflow_Subject implements DomainObject {
	public $_param1 = null;
	public $_param2 = null;

	static function getDefaultStorage()	{}
	function getInterfaces()			{}
	
	function isNull() {
		return false;
	}

	function callback1($param) {
		$this->_param1 = $param;
	}

	function callback2($param) {
		$this->_param2 = $param;
	}

	function getParent() {
		return SyndNodeLib::getInstance('null.null');
	}
}
