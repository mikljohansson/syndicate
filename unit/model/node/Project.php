<?php
require_once 'unit/SyndNodeTestCase.class.inc';

class _model_Node_Project extends PHPUnit2_Framework_TestCase {
	function testInputValidation() {
		$project = SyndNodeLib::factory('project');
		
		$errors = $project->validate(array('INFO_PROJECT_ID' => md5(uniqid(''))));	
		$this->assertEquals(array(), $errors);

		$errors = $project->validate(array('INFO_PROJECT_ID' => 'abc#¤%123'));	
		$this->assertEquals(array('INFO_PROJECT_ID'), array_keys($errors));

		$errors = $project->validate(array('INFO_PROJECT_ID' => '123'));	
		$this->assertEquals(array('INFO_PROJECT_ID'), array_keys($errors));
	}
	
	function testAttributes() {
		$project = SyndNodeLib::factory('project');
		$actual = $project->getAttribute('_unit_test');
		$this->assertNull($actual);
		
		$project->setAttribute('_unit_test', 'Test');
		$actual = $project->getAttribute('_unit_test');
		$this->assertEquals('Test', $actual);

		// Reload from storage
		$project->save();
		$project->flush();
		
		$persistent = $project->_storage->getPersistentStorage();
		$project2 = $persistent->getInstance($project->nodeId);
		$actual = $project2->getAttribute('_unit_test');
		$this->assertEquals('Test', $actual);

		// Test merge attributes
		$project->merge(array('attributes' => array('_unit_test2' => 'Test2')));
		$actual = $project->getAttribute('_unit_test2');
		$this->assertEquals('Test2', $actual);

		// Test inherit attributes
		$project3 = $project->appendChild($project->_storage->factory('project'));
		$actual = $project3->getAttribute('_unit_test');
		$this->assertEquals('Test', $actual);
		
		$project->delete();
	}
	
	function testEmailTemplates() {
		$template = file_get_contents(dirname(__FILE__).'/_project/project-001-template.txt');
		
		$project = SyndNodeLib::factory('project');
		$project->setTitle('_unit_test: This project is safe to delete');
		
		if (null == ($locale = SyndLib::runHook('getlocale')))
			$locale = 'en';
		$project->setTemplate('oncreate', $locale, $template);
		
		$issue = $project->appendChild($project->_storage->factory('issue'));
		$issue->setTitle('_unit_test: This issue is safe to delete');
		$issue->setDescription("Line 1\r\nLine 2\r\nLine 3");
		
		$assigned = SyndNodeLib::getInstance('user_case.Mikael Johansson');
		$assigned->setEmail('mikael@example.com');
		$issue->setAssigned($assigned);
		
		$customer = SyndNodeLib::getInstance('user_case.Some Customer');
		$customer->setEmail('customer@example.com');
		$issue->setCustomer($customer);
		
		$task = $issue->appendChild($issue->_storage->factory('task'));
		$task->setDescription('Latest note');
		
		$actual = $issue->getRenderedTemplate('oncreate', array($issue, '_callback_render_email'));
		
		$expected = file_get_contents(dirname(__FILE__).'/_project/project-001-expected.txt');
		$expected = str_replace('{$ID}', $issue->objectId(), $expected);
		$expected = str_replace('{$LINK}', tpl_request_host().tpl_view('issue',$issue->objectId(),$issue->getAuthenticationToken()), $expected);
		$expected = str_replace('{$FEEDBACK_LINK_YES}', tpl_request_host().tpl_view('issue','invoke',$issue->nodeId,'feedback',$issue->getAuthenticationToken(),1), $expected);
		$expected = str_replace('{$FEEDBACK_LINK_NO}', tpl_request_host().tpl_view('issue','invoke',$issue->nodeId,'feedback',$issue->getAuthenticationToken(),0), $expected);

		$this->assertEquals($expected, $actual);
		//$this->_diff($expected, $actual);
		
		// Test reload from database
		global $synd_config;
		$page = new Template(array_reverse($synd_config['dirs']['design']));
		
		$actual = $project->getTemplate($page, 'oncreate', $locale);
		$this->assertEquals($template, $actual);
		
		$project->save();
		$project->flush();
		
		$persistent = $project->_storage->getPersistentStorage();
		$project2 = $persistent->getInstance($project->nodeId);
		$actual = $project2->getTemplate($page, 'oncreate', $locale);
		$this->assertEquals($template, $actual);
		
		$project->delete();
	}
}
