<?php
require_once 'unit/SyndNodeTestCase.class.inc';

class _model_Node_Issue extends SyndNodeTestCase {
	function testAppendTask() {
		$issue = SyndNodeLib::factory('issue');
		$task = $issue->appendChild($issue->_storage->factory('task'));
		$this->assertType('object', $task);
		
		$tasks = $issue->getNotes();
		$this->assertEquals(1, count($tasks));
	}
	
	function testMergeTask() {
		$project = SyndNodeLib::factory('project');
		$issue = $project->_storage->factory('issue');
		$issue->appendChild($project->_storage->factory('task'));

		$tasks = $issue->getNotes();
		$this->assertEquals(1, count($tasks));
		
		$data = array(
			'PARENT_NODE_ID'	=> $project->nodeId,
			'INFO_HEAD' 		=> '_unit_test: This issue is safe to delete',
			'task' => array(
				'content'		=> '_unit_test: This task is safe to delete',
				),
			);

		$errors = $issue->validate($data);
		$this->assertEquals(array(), $errors);

		$issue->merge($data);
		$this->assertEquals($project, $issue->getParent());
		$this->assertEquals(2, count($issue->getNotes()));
		
		foreach ($issue->getNotes() as $task) {
			$task->delete();
			break;
		}

		$project->_storage->flush();
		$this->assertEquals(1, count($issue->getNotes()));

		// Test composite data (flipping between full_edit.tpl tabs)
		$data2 = $issue->getCompositeData();
		$this->assertTrue(isset($data2['task']));
		if (isset($data2['task']))
			$this->assertEquals($data['task']['content'], $data2['task']['content']);
	}

	function testAddRemoveCategories() {
		$category = SyndNodeLib::factory('keyword');
		$issue = SyndNodeLib::factory('issue');
	
		$issue->addCategory($category);
		$categories = $issue->getCategories();
		$this->assertEquals(1, count($categories));
		
		$issue->removeCategory($category);
		$categories = $issue->getCategories();
		$this->assertEquals(0, count($categories));
	}
	
	function testMergeCategories() {
		$storage = SyndNodeLib::getDefaultStorage('issue');
		$database = $storage->getDatabase();

		$sql = "
			DELETE FROM synd_project
			WHERE info_head = '_unit_test: This issue is safe to delete'";
		$database->query($sql);

		$sql = "
			DELETE FROM synd_keyword
			WHERE info_head = '_unit_test: This keyword is safe to delete'";
		$database->query($sql);

		$sql = "
			DELETE FROM synd_issue
			WHERE info_head = '_unit_test: This issue is safe to delete'";
		$database->query($sql);

		$project = $storage->factory('project');
		$project->setTitle('_unit_test: This project is safe to delete');
		
		$category1 = $project->appendChild($storage->factory('keyword'));
		$category1->setTitle('_unit_test: This keyword is safe to delete');
		
		$category2 = $project->appendChild($storage->factory('keyword'));
		$category2->setTitle('_unit_test: This keyword is safe to delete');

		$issue = $project->appendChild($storage->factory('issue'));
		$issue->setTitle('_unit_test: This issue is safe to delete');
		$issue->addCategory($category1);

		// Test assigning and removing keyword
		$data = array(
			'categories' => array(
				'previous' => SyndLib::collect($issue->getCategories(),'nodeId'),
				'selected' => array($category2->nodeId),
				),
			);
		$issue->merge($data);
		
		$categories = SyndLib::collect($issue->getCategories(),'nodeId');
		$this->assertFalse(in_array($category1->nodeId, $categories));
		$this->assertTrue(in_array($category2->nodeId, $categories));
		
		// Test partial assign
		$data = array(
			'categories' => array(
				'selected' => array($category1->nodeId),
				),
			);
		$issue->merge($data);
		
		$categories = SyndLib::collect($issue->getCategories(),'nodeId');
		$this->assertTrue(in_array($category1->nodeId, $categories));
		$this->assertTrue(in_array($category2->nodeId, $categories));
		
		// Test reload from database
		$project->save();
		$category1->save();
		$category2->save();
		$issue->save();
		$issue->flush();

		$persistent = $storage->getPersistentStorage(); 
		$issue2 = $persistent->getInstance($issue->nodeId);

		$categories = SyndLib::collect($issue2->getCategories(),'nodeId');
		$this->assertTrue(in_array($category1->nodeId, $categories));
		$this->assertTrue(in_array($category2->nodeId, $categories));
		
		$issue->delete();
		$category1->delete();
		$category2->delete();
		$project->delete();
	}
	
	function testAttachFile() {
		global $synd_config;
		$buffer = 'test';
		$path = $synd_config['dirs']['cache'].'_unit_test';
		SyndLib::createDirectory($synd_config['dirs']['cache']);
		SyndLib::file_put_contents($path, $buffer);
		$this->assertEquals(file_get_contents($path), $buffer);

		$issue = SyndNodeLib::factory('issue');
		$issue->appendChild(SyndType::factory('file', $path, '_unit_test_file'));
		
		$files = $issue->getFiles();
		if (!empty($files)) {
			$this->assertEquals('_unit_test_file', reset($files)->toString());
			$this->assertEquals($buffer, reset($files)->getContents());
		}
		
		$issue->delete();
		unlink($path);
	}
	
	function testAutenticatenToken() {
		$issue = SyndNodeLib::factory('issue');
		$this->assertTrue($issue->validateAuthenticationToken($issue->getAuthenticationToken()));
	}
	
	function testDuration() {
		$issue = SyndNodeLib::factory('issue');
		$project = SyndNodeLib::factory('project');
		
		$data = array(
			'PARENT_NODE_ID' => $project->nodeId,
			'INFO_HEAD' => '_unit_test: This issue is safe to delete',
			'INFO_ESTIMATE' => '60*2',
			'task' => array(
				'content' => '_unit_test: This task is safe to delete',
				'INFO_DURATION' => '60*3+20',
				),
			);
			
		$errors = $issue->validate($data);
		$this->assertEquals(array(), $errors);
		
		$issue->merge($data);
		$this->assertEquals((string)(60*2*60), $issue->getEstimate());
		
		$tasks = $issue->getNotes();
		$this->assertEquals(1, count($tasks));

		$actual = SyndLib::invoke($tasks, 'getDuration');
		$this->assertEquals(array((string)((60*3+20)*60)), $actual);
	}
	
	function testAttributes() {
		$project = SyndNodeLib::factory('project');
		$project->setTitle('_unit_test: This project is safe to delete');
		$project->save();
		
		$issue = $project->appendChild($project->_storage->factory('issue'));
		$issue->setTitle('_unit_test: This issue is safe to delete');

		$actual = $issue->getAttribute('_unit_test');
		$this->assertNull($actual);
		
		$issue->setAttribute('_unit_test', 'Test');
		$actual = $issue->getAttribute('_unit_test');
		$this->assertEquals('Test', $actual);

		// Reload from storage
		$issue->save();
		$issue->flush();
		
		$persistent = $issue->_storage->getPersistentStorage();
		$issue2 = $persistent->getInstance($issue->nodeId);
		
		$actual = $issue2->getAttribute('_unit_test');
		$this->assertEquals('Test', $actual);

		$issue->delete();
		$project->delete();
	}
	
	function testSerialization() {
		$issue = SyndNodeLib::factory('issue');
		$expected = 'Test';

		$issue->setDescription($expected);
		$issue->merge(array('task' => array('content' => 'Test2', 'INFO_DURATION' => '20')));
		
		$actual = $issue->getDescription();
		$this->assertEquals($expected, $actual);
		$tasks = $issue->getNotes();
		$this->assertEquals(1, count($tasks));

		$issue2 = unserialize(serialize($issue));
		
		$actual = $issue2->getDescription();
		$this->assertEquals($expected, $actual);
		$tasks = $issue2->getNotes();
		$this->assertEquals(1, count($tasks));
	}
	
	function testActivateLinks(){
		$issue = SyndNodeLib::factory('issue');
		$issue->setTitle('_unit_test: This issue is safe to delete "&');
		$issue->setDescription('Test #'.$issue->objectId().' a link');
		$this->assertEquals(
			'Test <a href="'.tpl_link('issue', $issue->objectId()).'" title="'.htmlspecialchars($issue->getTitle()).'">#'.$issue->objectId().'</a> a link', 
			trim(strip_tags(tpl_gui_node($issue->getContent(),'full_view.tpl',array('filter' => array($issue, '_callback_filter'))), '<a>')));
	}
	
	function testEmailSenders() {
		$project = SyndNodeLib::factory('project');
		$project->setTitle('_unit_test');
		$project->setEmail('example@example.com');
		$project->data['FLAG_DISPLAY_SENDER'] = 0;
		$issue = $project->appendChild($project->_storage->factory('issue'));

		$expected = array('"_unit_test" <example+'.$issue->objectId().'@example.com>');
		$actual = $issue->getEmailSenders();
		$this->assertEquals($expected, $actual);
	}
	
	function testWordwrap() {
		$expected = file_get_contents(dirname(__FILE__).'/_issue/issue-001-wordwrap-expected.txt');
		$buffer = file_get_contents(dirname(__FILE__).'/_issue/issue-001-wordwrap.txt');
		$actual = synd_node_issue::wordwrap($buffer, 48);
		$this->assertEquals($expected, $actual);
	}
}
