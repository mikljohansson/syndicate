<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _plugin_Crypto extends PHPUnit2_Framework_TestCase {
	protected $_plugin = null;
	
	function setUp() {
		require_once 'core/lib/crypto/driver/pgp.class.inc';
		$issue = Module::getInstance('issue');
		$this->_plugin = $issue->loadPlugin('crypto');
	}
	
	function testVerbatim() {
		$storage = SyndNodeLib::getDefaultStorage('project');
		$database = $storage->getDatabase();
		
		$sql = "
			DELETE FROM synd_issue
			WHERE 
				parent_node_id IN (
					SELECT node_id FROM synd_project
					WHERE info_head = '_unit_test: This project is safe to delete')";
		$database->query($sql);
		
		$sql = "
			DELETE FROM synd_project
			WHERE info_head = '_unit_test: This project is safe to delete'";
		$database->query($sql);

		$project = $storage->factory('project');
		$project->setParent(SyndNodeLib::getInstance('null._unit_test'));
		$project->setTitle('_unit_test: This project is safe to delete');
		$project->save();
		
		$buffer = file_get_contents(dirname(__FILE__).'/_crypto/pgp-001-signed-mime.msg');

		$module = Module::getInstance('issue');
		$module->_mail(array($project->nodeId), $buffer);
		
		// Find issue
		$sql = "
			SELECT i.node_id FROM synd_issue i
			WHERE i.client_node_id = 'user_case.mikael@example.com'";
		$issues = $storage->getInstances($database->getCol($sql));
		$issue = reset($issues);
		$this->assertNotNull($issue);

		$mime = $issue->getContent()->getMessage();
		$parts = $mime->getParts();
		$this->assertEquals(2, count($parts));

		$expected = file_get_contents(dirname(__FILE__).'/_crypto/pgp-001-signed-body.msg');
		$actual = $parts[0]->toString();
		$this->assertEquals($expected, $actual);

		$issue->delete();
		$project->delete();
		$storage->flush();
	}
}
