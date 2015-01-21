<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _plugin_Svn extends PHPUnit2_Framework_TestCase {
	function setUp() {
		$issue = Module::getInstance('issue');
		$issue->loadPlugin('svn');
	}
	
	function testAppendRevision() {
		$project = SyndNodeLib::factory('project');
		$project->setAttribute('svn_repository_location', 'http://svn.synd.info/synd/');
		$issue = $project->appendChild($project->_storage->factory('issue'));
		$issue->setTitle('_unit_test: This issue is safe to delete');

		// Test validatation
		$errors = $issue->validate(array('svn' => array('revision' => 'abc')));
		$this->assertTrue(isset($errors['svn_revision']));
		$errors = $issue->validate(array('svn' => array('revision' => '123')));
		$this->assertFalse(isset($errors['svn_revision']));
		
		// Test merge
		$issue->merge(array('svn' => array('revision' => '123')));
		$issue->merge(array('svn' => array('revision' => '456')));
		$tasks = $issue->getNotes();
		$this->assertEquals(1, count($tasks));
		
		if (!empty($tasks)) {
			$content = $tasks[key($tasks)]->getContent();
			$this->assertEquals('456', $content->toString());
		}
		
		// Test flush and reload
		$project->save();
		$issue->save();
		$issue->flush();
		
		$persistent = $issue->_storage->getPersistentStorage();
		$issue2 = $persistent->getInstance($issue->nodeId);
		$tasks2 = $issue2->getNotes();
		$this->assertEquals(1, count($tasks2));
		
		if (!empty($tasks2)) {
			$content = $tasks2[key($tasks)]->getContent();
			$this->assertEquals('456', $content->toString());
		}
		
		$issue->delete();
		$project->delete();
	}
}