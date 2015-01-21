<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _modules_IssueSyncronize extends PHPUnit2_Framework_TestCase {
	function setUp() {
		require_once 'core/module/issue/SyncableIssueCollection.class.inc';
		require_once 'core/lib/calendar/VCalendar.class.inc';
		require_once 'core/model/syncml/message.class.inc';
	}
	
	function testSyncronize() {
		global $synd_user;
		
		$sql = "
			DELETE FROM synd_issue 
			WHERE assigned_node_id = 'user_case._unit_test'";
		$project = $this->_project();
		$project->_db->query($sql);
		
		Module::getInstance('issue');
		$user = SyndNodeLib::getInstance('user_case._unit_test');
		
		$keyword = SyndNodeLib::factory('keyword');
		$keyword->setParent($project);
		$keyword->setTitle(strtoupper(md5(uniqid(''))));
		$keyword->save();
		$keyword->flush();
		
		$issue = $this->_issue($project);
		$issue->setAssigned($user);
		$issue->flush();
		
		$collections = SyndLib::runHook('syncronize', 'tasks', array($user), null);
		$this->assertNotNull($collections);
		if (null != $collections) {
			$collection = $collections[key($collections)];
			$instance = $collection->getInstance($issue->id());
			$expected = array($issue->id() => $instance);
			$actual = $collection->getContents();
			$this->assertEquals($expected, $actual);

			// Test syncing the issue
			$buffer = file_get_contents(dirname(__FILE__).'/_issue/memento-001.ics');
			$buffer = str_replace('__KEYWORD__', strtolower($keyword->toString()), $buffer);
			
			$memento = VCalendar::parse($buffer);
			$instance = $collection->getInstance($issue->id());
			$instance->setMemento($memento);

			$todo = reset($memento->getChildren());
			$this->assertEquals($todo->getSummary(), $issue->getTitle());
			$this->assertEquals($todo->getDescription(), $issue->getDescription());
			$this->assertEquals(3 - $todo->getProperty('PRIORITY'), $issue->data['INFO_PRIO']);

			$this->assertEquals(strtotime('2005-01-18 00:00:00'), $issue->data['TS_RESOLVE_BY']);
			$this->assertEquals(strtotime('2005-01-19 00:00:00'), $issue->data['TS_RESOLVE']);

			// Check if keyword has been set
			$keywords = $issue->getCategories();
			$this->assertEquals(1, count($keywords));
			$this->assertTrue(in_array($keyword->nodeId, SyndLib::collect($keywords,'nodeId')));
			
			$instance = $collection->remove($instance);
			$this->assertEquals($issue, $instance->_issue);
			$this->assertTrue($issue->isClosed());
		}
		
		$issue->_mailNotifier->_active = array();
		$issue->delete();
		$keyword->delete();
		$project->_mailNotifier->_active = array();
		$project->delete();
		$project->_storage->flush();
	}
	
	function testSyncronizeAppend() {
		$sql = "
			DELETE FROM synd_issue 
			WHERE assigned_node_id = 'user_case._unit_test'";
		$project = $this->_project();
		$project->_db->query($sql);

		Module::getInstance('issue');
		$user = SyndNodeLib::getInstance('user_case._unit_test');
		
		$collections = SyndLib::runHook('syncronize', 'tasks', array($user), null);
		$this->assertNotNull($collections);
		if (null == $collections) return;

		$collection = $collections[key($collections)];
		$collection->_project = $project->id();
		$this->assertEquals(array(), $collection->getContents());
		
		$memento = VCalendar::parse(file_get_contents(dirname(__FILE__).'/_issue/memento-001.ics'));
		$instance = $collection->append($memento);
		$this->assertNotNull($instance);

		if (null != $instance) {
			$todo = reset($memento->getChildren());

			$this->assertEquals($todo->getSummary(), $instance->_issue->getTitle());
			$this->assertEquals($todo->getDescription(), $instance->_issue->getDescription());
			$this->assertEquals(3 - $todo->getProperty('PRIORITY'), $instance->_issue->data['INFO_PRIO']);

			$this->assertEquals(strtotime('2005-01-18 00:00:00'), $instance->_issue->data['TS_RESOLVE_BY']);
			$this->assertEquals(strtotime('2005-01-19 00:00:00'), $instance->_issue->data['TS_RESOLVE']);

			$this->assertTrue($instance->_issue->isDirty() || !$instance->_issue->isNew());
			
			$this->assertTrue(is_array($instance->_issue->_dirtyCategories));
			$this->assertTrue(empty($instance->_issue->_dirtyCategories));

			$instance->_issue->_mailNotifier->_active = array();
			$instance->_issue->delete();
		}
		
		$project->_mailNotifier->_active = array();
		$project->delete();
		$project->_storage->flush();
	}
	
	function testNotes() {
		$project = $this->_project();
		$issue = $this->_issue($project);
		
		$task = $issue->_storage->factory('task');
		$task->setParent($issue);
		$task->setCreator(SyndNodeLib::getInstance('user_case._unit_test'));
		$task->setDescription($id = md5(uniqid('')));
		$task->setDuration(200*60);
		$task->save();
		$task->flush();
		
		$task2 = $issue->_storage->factory('task');
		$task2->setParent($issue);
		$task2->setCreator(SyndNodeLib::getInstance('user_case._unit_test'));
		$task2->setDescription($id2 = md5(uniqid('')));
		$task2->setDuration(30*60);
		$task2->save();
		$task2->flush();
		
		$collection = new SyncableIssueTodoCollection(
			SyndType::factory('null_collection'), $project, 
			SyndNodeLib::getInstance('user_case._unit_test'));
			
		$instance = new SyncableTodoIssue($collection, $issue);
		$memento = $instance->getMemento();
		$children = $memento->getChildren();
		$vtodo = $children[key($children)];
		
		$this->assertTrue(false !== strpos($vtodo->getDescription(), $task->nodeId));
		$this->assertTrue(false !== strpos($vtodo->getDescription(), $id));
		$this->assertEquals($id, $task->getDescription());
		$this->assertEquals(200*60, $task->getDuration());
		$this->assertEquals($id2, $task2->getDescription());
		$this->assertEquals(30*60, $task2->getDuration());
		
		$description = $vtodo->getDescription();
		$description = str_replace('_unit_test: Test issue', $newDescription = 'New issue description', $description);
		$description = str_replace($id, $idNew = md5(uniqid('')), $description);
		$description = str_replace('minutes="200"', 'minutes="40"', $description);
		$description = str_replace($id2, $id2New = md5(uniqid('')), $description);
		$description = str_replace('minutes="30"', 'minutes="50"', $description);
		$description .= "

<synd:note minutes=\"45\">
New note
</synd:note>";

		$vtodo->replaceProperty('DESCRIPTION', $description);
		$instance->setMemento($memento);
		
		// Assert that the new issue description has been extracted and set
		$this->assertEquals($newDescription, $issue->getDescription());

		// Assert that notes have been changed
		$this->assertEquals($idNew, $task->getDescription());
		$this->assertEquals(40*60, $task->getDuration());
		$this->assertEquals($id2New, $task2->getDescription());
		$this->assertEquals(50*60, $task2->getDuration());
		
		// Assert that the new note has been created
		$tasks = $issue->getNotes();
		$this->assertEquals(3, count($tasks));
		$this->assertTrue(in_array('New note', SyndLib::invoke($tasks, 'getDescription')));
		$this->assertTrue(in_array(45*60, SyndLib::invoke($tasks, 'getDuration')));
		
		$issue->_mailNotifier->_active = array();
		$issue->delete();
		$project->_mailNotifier->_active = array();
		$project->delete();
		$project->_storage->flush();
	}
	
	function testWhitespaceInsignificance() {
		$collection = new SyncableIssueTodoCollection(
			SyndType::factory('null_collection'), 
			SyndNodeLib::getInstance('null.null'), 
			SyndNodeLib::getInstance('user_null.null'));
		
		$expected = "abc\r\nabc";
		
		$issue = SyndNodeLib::factory('issue');
		$issue->setDescription($expected);
		$syncable = new SyncableTodoIssue($collection, $issue);
		
		// Description shouldn't change
		$memento = new VCalendarObject();
		$memento->appendChild(new VCalendarTodo(null, null, '_unit_test: This issue is safe to delete', "abc\r\n\r\nabc"));
		$syncable->setMemento($memento);
		$this->assertEquals($expected, $issue->getDescription());

		// Description should change
		$expected = "abc\r\n\r\nabc\r\n123";
		$memento = new VCalendarObject();
		$memento->appendChild(new VCalendarTodo(null, null, '_unit_test: This issue is safe to delete', $expected));
		$syncable->setMemento($memento);
		$this->assertEquals($expected, $issue->getDescription());
	}
	
	function _project() {
		$project = SyndNodeLib::factory('project');
		$project->setParent(SyndNodeLib::getInstance('null._unit_test'));
		$project->setTitle('_unit_test: This project is safe to delete');
		$project->save();
		$project->flush();
		return $project;
	}
	
	function _issue($project) {
		$issue = $project->_storage->factory('issue');
		$issue->setParent($project);
		$issue->setAssigned(SyndNodeLib::getInstance('user_case._unit_test'));
		$issue->setCustomer(SyndNodeLib::getInstance('user_case._unit_test'));
		$issue->setTitle('_unit_test: This issue is safe to delete');
		$issue->setDescription('_unit_test: Test issue');
		$issue->save();
		$issue->flush();
		return $issue;
	}
}

