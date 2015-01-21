<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _modules_Issue extends PHPUnit2_Framework_TestCase {
	function testCreateMail() {
		$storage = SyndNodeLib::getDefaultStorage('project');
		$database = $storage->getDatabase();
		
		$project1 = $storage->factory('project');
		$project1->setParent(SyndNodeLib::getInstance('null._unit_test'));
		$project1->setTitle('_unit_test: This project is safe to delete');

		$project2 = $project1->appendChild($storage->factory('project'));
		$project2->setTitle('_unit_test: This project is safe to delete');
		$project2->data['INFO_PROJECT_ID'] = md5(uniqid(''));

		$project1->save();
		$project2->save();
		$storage->flush();

		$sql = "
			DELETE FROM synd_issue 
			WHERE 
				client_node_id = 'user_case.mikael@example.com' OR 
				client_node_id = 'user_case._unit_test'";
		$project1->_db->query($sql);

		// Receive faked email
		$subject = '_unit_test: This issue is safe to delete';
		$buffer = file_get_contents(dirname(__FILE__).'/_issue/mail-005-attachment.msg');
		$buffer = str_replace('__PROJECT__', $project2->data['INFO_PROJECT_ID'], $buffer);
		$buffer = str_replace('__SUBJECT__', $subject, $buffer);

		$result = Module::getInstance('issue')->_mail(array($project1->nodeId), $buffer);
		$this->assertTrue($result);
		
		// Find issue
		$sql = "
			SELECT i.node_id FROM synd_issue i
			WHERE i.client_node_id = 'user_case.mikael@example.com'";
		$issues = $storage->getInstances($database->getCol($sql));
		$this->assertFalse(empty($issues));
		$issue = reset($issues);

		$this->assertEquals($subject, $issue->getTitle());
		$this->assertEquals('test', trim($issue->getDescription()));

		$project = $issue->getParent();
		$this->assertEquals($project2->nodeId, $project->nodeId);
		$this->assertEquals('mikael@example.com', $issue->data['INFO_INITIAL_QUERY']);
		
		$files = $issue->getFiles();
		$file = reset($files);
		$this->assertEquals('object', gettype($file));
			
		$this->assertEquals('foo.txt', $file->toString());
		$this->assertEquals('foo', trim($file->getContents()));

		$issue->delete();
		
		$project1->_mailNotifier->_active = array();
		$project1->delete();

		$project2->_mailNotifier->_active = array();
		$project2->delete();
	}
	
	function testInitialMapping() {
		$storage = SyndNodeLib::getDefaultStorage('project');
		$database = $storage->getDatabase();
		
		$project = $storage->factory('project');
		$project->setParent(SyndNodeLib::getInstance('null._unit_test'));
		$project->setTitle('_unit_test: This project is safe to delete');
		$project->save();
		$project->flush();

		$sql = "
			DELETE FROM synd_issue 
			WHERE 
				client_node_id = 'user_case.mikael@example.com' OR 
				client_node_id = 'user_case._unit_test'";
		$project->_db->query($sql);

		// Setup mapping
		$result = $project->_db->replace('synd_project_mapping', array(
			'PROJECT_NODE_ID'	=> $project->nodeId,
			'CUSTOMER_NODE_ID'	=> 'node.user_case._unit_test',
			'QUERY'				=> 'mikael@example.com'));
		
		// Receive faked email
		$subject = '_unit_test: This issue is safe to delete';
		$buffer = file_get_contents(dirname(__FILE__).'/_issue/mail-001-unassigned.msg');
		$buffer = str_replace('__PROJECT__', $project->data['INFO_PROJECT_ID'], $buffer);
		
		$result = Module::getInstance('issue')->_mail(array($project->nodeId), $buffer);
		$this->assertTrue($result);

		// Find issue
		$sql = "
			SELECT i.node_id FROM synd_issue i
			WHERE i.client_node_id = 'user_case._unit_test'";
		$issues = $storage->getInstances($database->getCol($sql));
		$this->assertFalse(empty($issues));
		$issue = reset($issues);
		
		$this->assertEquals('user_case._unit_test', $issue->getCustomer()->nodeId);
		
		$issue->delete();
		$project->delete();
	}
	
	function testAppendMail() {
		$module = Module::getInstance('issue');
		$storage = SyndNodeLib::getDefaultStorage('project');
		$persistent = $storage->getPersistentStorage();
		
		$project = $persistent->factory('project');
		$project->setParent(SyndNodeLib::getInstance('null._unit_test'));
		$project->setTitle('_unit_test: This project is safe to delete');
		$project->save();
		
		$issue = $project->appendChild($project->_storage->factory('issue'));
		$issue->setCustomer(SyndNodeLib::getInstance('user_case._unit_test'));
		$issue->setTitle('_unit_test: This issue is safe to delete');
		$issue->save();
		$project->_storage->flush();

		$subject = '_unit_test: This issue is safe to delete #'.$issue->objectId();
		if (null != $module->getNamespace())
			$subject .= '@'.$module->getNamespace();
		$email = md5(uniqid('')).'@example.com';
		
		$buffer = file_get_contents(dirname(__FILE__).'/_issue/mail-005-attachment.msg');
		$buffer = str_replace('__SUBJECT__', $subject, $buffer);
		$buffer = str_replace('mikael@example.com', $email, $buffer);

		$this->assertNotEquals(false, strpos($buffer, $subject));
		
		require_once 'core/lib/HttpConnection.class.inc';
		$connection = new HttpConnection(tpl_request_host());
		$result = $connection->postRequest(tpl_view('issue','mail',$project->nodeId), $buffer, 'text/plain');
		
		// Reload issue from database and check content
		$reloaded = $persistent->getInstance($issue->nodeId);
		$this->assertNotNull($reloaded);
		
		$notes = $reloaded->getNotes();
		$this->assertEquals(1, count($notes));
		
		foreach ($notes as $note) {
			$files = $note->getFiles();
			$file = reset($files);
			$this->assertEquals('object', gettype($file));

			$this->assertEquals('foo.txt', $file->toString());
			$this->assertEquals('foo', trim($file->getContents()));

			$this->assertEquals(SyndNodeLib::getInstance('user_case.'.$email)->nodeId, $note->getCreator()->nodeId);
		}
		
		// Check that task was appended and written to storage
		$sql = "
			SELECT COUNT(*) FROM synd_issue_task t
			WHERE t.parent_node_id = ".$issue->_db->quote($issue->nodeId);
		$actual = $issue->_db->getOne($sql);
		$this->assertEquals(1, $actual);

		$issue->delete();
		
		// Test example+12345@example.com style issue addressing
		require_once 'core/lib/Mime.class.inc';
		$module = Module::getInstance('issue');
		$storage = SyndNodeLib::getDefaultStorage('issue');
		$issue = $project->appendChild($storage->factory('issue'));
		$issue->setTitle('_unit_test: This issue is safe to delete');
		
		$issue->save();
		$issue->flush();
		
		// Test To: addressing
		$mime = new MimeTextpart('Test');
		$mime->setHeader('To', 'example+'.$issue->objectId().'@example.com');
		$mime->setHeader('From', 'example@example.com');
		
		$module->_mail(array($project->nodeId), $mime->toString());
		$notes = $issue->getNotes();
		$this->assertEquals(1, count($notes));
		
		foreach ($notes as $note) {
			$this->assertEquals('Test', $note->getDescription());
			$this->assertEquals('example@example.com', $note->getCreator()->getEmail());
		}
		
		// Test Cc: addressing
		$mime = new MimeTextpart('Test');
		$mime->setHeader('Cc', 'example+'.$issue->objectId().'@example.com');

		$module->_mail(array($project->nodeId), $mime->toString());
		$notes = $issue->getNotes();
		$this->assertEquals(2, count($notes));

		// Test Delivered-To: addressing
		$mime = new MimeTextpart('Test');
		$mime->setHeader('Delivered-To', 'u-example+'.$issue->objectId().'@example.com');

		$module->_mail(array($project->nodeId), $mime->toString());
		$notes = $issue->getNotes();
		$this->assertEquals(3, count($notes));

		// Test X-Original-To: addressing
		$mime = new MimeTextpart('Test');
		$mime->setHeader('X-Original-To', 'example+'.$issue->objectId().'@example.com');

		$module->_mail(array($project->nodeId), $mime->toString());
		$notes = $issue->getNotes();
		$this->assertEquals(4, count($notes));

		// Clear out created nodes
		$issue->delete();
		$project->delete();
		$project->flush();
	}
	
	function testFindProject() {
		$project = SyndNodeLib::factory('project');
		$sql = "
			DELETE FROM synd_project
			WHERE info_project_id = '_unit_test'";
		$project->_db->query($sql);

		$project->data['INFO_HEAD'] = '_unit_test';
		$project->data['INFO_PROJECT_ID'] = '_unit_test';
		$project->save();
		$project->flush();

		$module = Module::getInstance('issue');
		
		$mail = Mime::parse(file_get_contents(dirname(__FILE__).'/_issue/mail-006.msg'));
		$actual = $module->_findProject($project->_storage, $project, $mail);
		$this->assertEquals($project->nodeId, $actual->nodeId);

		$mail = Mime::parse(file_get_contents(dirname(__FILE__).'/_issue/mail-007.msg'));
		$actual = $module->_findProject($project->_storage, $project, $mail);
		$this->assertNotNull($actual);
		$this->assertEquals($project->nodeId, $actual->nodeId);

		$mail = Mime::parse(file_get_contents(dirname(__FILE__).'/_issue/mail-001-unassigned.msg'));
		$actual = $module->_findProject($project->_storage, $project, $mail);
		$this->assertNull($actual);

		$mail = Mime::parse(file_get_contents(dirname(__FILE__).'/_issue/mail-008-project-cc.msg'));
		$actual = $module->_findProject($project->_storage, $project, $mail);
		$this->assertNotNull($actual);
		$this->assertEquals($project->nodeId, $actual->nodeId);

		$project->delete();
	}
	
	function testFindClient() {
		$sender = SyndNodeLib::getInstance('user_case._unit_test');
		$module = Module::getInstance('issue');
		$project = SyndNodeLib::factory('project');
		
		$actual = $module->_findClient($project, $sender);
		$this->assertEquals($sender->nodeId, $actual->nodeId);

		$user = SyndNodeLib::getInstance('user_case._unit_test2');
		$project->data['INFO_DEFAULT_CLIENT'] = $user->objectId();
		$actual = $module->_findClient($project, $user);
		
		$this->assertEquals($user->nodeId, $actual->nodeId);
	}
	
	function testMergePrimary() {
		$module = Module::getInstance('issue');
		
		$storage = SyndNodeLib::getDefaultStorage('project');
		$project = $storage->factory('project');
		$project->setTitle('_unit_test: This project is safe to delete');
		$project->save();

		// Issues to be merged
		$issue1 = $project->appendChild($storage->factory('issue'));
		$issue1->setTitle('_unit_test: This issue is safe to delete');
		$issue1->save();
		$issue2 = $project->appendChild($storage->factory('issue'));
		$issue2->setTitle('_unit_test: This issue is safe to delete');
		$issue2->save();
		$storage->flush();
	
		// Merge issue2 to primary issue1
		$issue = $module->_getMergedPrimary($issue1->nodeId, array($issue1, $issue2));
		$this->assertEquals($issue1->nodeId, $issue->nodeId);
		
		$subissues = iterator_to_array($issue->getChildren());
		$subissue = reset($subissues);

		$this->assertEquals(1, count($subissues));
		$this->assertEquals($issue2->nodeId, $subissue->nodeId);
		$this->assertEquals(synd_node_issue::CLOSED, $subissue->data['INFO_STATUS']);
		
		// Test mail bubbling to primary issue
		$notes = $issue1->getNotes();
		$this->assertEquals(0, count($notes));

		$subject = '_unit_test: This issue is safe to delete #'.$issue2->objectId();
		if (null != $module->getNamespace())
			$subject .= '@'.$module->getNamespace();
		$email = md5(uniqid('')).'@example.com';
		
		$buffer = file_get_contents(dirname(__FILE__).'/_issue/mail-005-attachment.msg');
		$buffer = str_replace('__SUBJECT__', $subject, $buffer);
		$buffer = str_replace('mikael@example.com', $email, $buffer);

		$this->assertNotEquals(false, strpos($buffer, $subject));
		
		$module = Module::getInstance('issue');
		$result = $module->_mail(array($project->nodeId), $buffer);
		$this->assertTrue($result);
		
		$notes = $issue1->getNotes();
		$this->assertEquals(1, count($notes));
		foreach ($notes as $note)
			$this->assertTrue(false !== strpos($note->getContent()->getSender(), $email));
		
		// Reload from database
		$issue->save();
		SyndLib::invoke($subissues, 'save');
		$storage->flush();
		
		$persistent = $storage->getPersistentStorage();
		$reloaded = $persistent->getInstance($issue->nodeId);
		$this->assertNotNull($reloaded);
		
		if (null != $reloaded) {
			$this->assertEquals($issue->nodeId, $reloaded->nodeId);
			$subissues = iterator_to_array($issue->getChildren());
			$subissue = $subissues[key($subissues)];

			$this->assertEquals(1, count($subissues));
			$this->assertEquals($issue2->nodeId, $subissue->nodeId);
			$this->assertEquals(synd_node_issue::CLOSED, $subissue->data['INFO_STATUS']);
		}		
				
		$issue1->delete();
		$issue2->delete();
		$issue->delete();
		$project->delete();
	}

	function testMergePrototype() {
		$module = Module::getInstance('issue');
		
		$storage = SyndNodeLib::getDefaultStorage('project');
		$project = $storage->factory('project');
		$project->setTitle('_unit_test: This project is safe to delete');
		$project->save();

		// Issues to be merged
		$issue1 = $project->appendChild($storage->factory('issue'));
		$issue1->setTitle('_unit_test: This issue is safe to delete');
		$issue1->save();
		$issue2 = $project->appendChild($storage->factory('issue'));
		$issue2->setTitle('_unit_test: This issue is safe to delete');
		$issue2->save();
		$storage->flush();
	
		// Merge issues to new from prototype
		$issue = $module->_getMergedPrimary('new', array($issue1, $issue2));
		$subissues = iterator_to_array($issue->getChildren());
		$subissue = $subissues[key($subissues)];

		$this->assertEquals(2, count($subissues));
		$this->assertEquals(synd_node_issue::CLOSED, $subissue->data['INFO_STATUS']);

		// Reload from database
		$issue->save();
		SyndLib::invoke($subissues, 'save');
		$issue->flush();
		
		$persistent = $storage->getPersistentStorage();
		$reloaded = $persistent->getInstance($issue->nodeId);
		$this->assertNotNull($reloaded);
		
		$this->assertEquals($issue->nodeId, $reloaded->nodeId);
		$subissues = iterator_to_array($reloaded->getChildren());
		$subissue = $subissues[key($subissues)];

		$this->assertEquals(2, count($subissues));
		$this->assertEquals(synd_node_issue::CLOSED, $subissue->data['INFO_STATUS']);

		$issue1->delete();
		$issue2->delete();
		$issue->delete();
		$project->delete();
	}
	
	function testSpamFilter() {
		$module = Module::getInstance('issue');
		
		$storage = SyndNodeLib::getDefaultStorage('project');
		$project = $storage->factory('project');
		$project->setTitle('_unit_test: This project is safe to delete');
		$project->data['INFO_PROJECT_ID'] = md5(uniqid(''));
		$project->data['FLAG_DISCARD_SPAM'] = 0;
		$project->save();
		$project->flush();

		$sql = "
			DELETE FROM synd_issue 
			WHERE 
				client_node_id = 'user_case.mikael@example.com' OR 
				client_node_id = 'user_case._unit_test'";
		$project->_db->query($sql);

		// Receive faked email
		$subject = "***SPAM*** _unit_test: This issue is safe to delete ".md5(uniqid(''));
		$buffer = file_get_contents(dirname(__FILE__).'/_issue/mail-010-spam.msg');
		$buffer = str_replace('__PROJECT__', $project->data['INFO_PROJECT_ID'], $buffer);
		$buffer = str_replace('__SUBJECT__', $subject, $buffer);
		
		$result = $module->_mail(array($project->nodeId), $buffer);
		$this->assertTrue($result);
		
		// Sync new issue to database
		SyndLib::runHook('shutdown');
		
		// Find issue
		$sql = "
			SELECT i.node_id FROM synd_issue i
			WHERE i.client_node_id = 'user_case.mikael@example.com'";
		$issues = $storage->getInstances($project->_db->getCol($sql));
		$this->assertFalse(empty($issues));
		$issue = reset($issues);
		
		$this->assertEquals($subject, $issue->getTitle());
		$issue->delete();
		
		// Test with discard spam flag
		$project->data['FLAG_DISCARD_SPAM'] = 1;
		$project->save();
		$project->flush();

		// Receive faked email
		$subject = "***SPAM*** _unit_test: This issue is safe to delete ".md5(uniqid(''));
		$buffer = file_get_contents(dirname(__FILE__).'/_issue/mail-010-spam.msg');
		$buffer = str_replace('__PROJECT__', $project->data['INFO_PROJECT_ID'], $buffer);
		$buffer = str_replace('__SUBJECT__', $subject, $buffer);
		
		$result = $module->_mail(array($project->nodeId), $buffer);
		$this->assertTrue($result);
		
		// Sync new issue to database
		SyndLib::runHook('shutdown');
		
		// Find issue
		$sql = "
			SELECT i.node_id FROM synd_issue i
			WHERE i.client_node_id = 'user_case.mikael@example.com'";
		$issues = $storage->getInstances($project->_db->getCol($sql));
		$this->assertTrue(empty($issues));
	}
}
