<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_DatabaseEntity extends PHPUnit2_Framework_TestCase {
	function setUp() {
		require_once 'core/model/DatabaseEntity.class.inc';
	}
	
	function testEntityQuery() {
		$storage = SyndNodeLib::getDefaultStorage('unit_test');
		$node1 = $storage->factory('unit_test');
		$node2 = $storage->factory('unit_test');
		
		$node1->save();
		$node2->save();
		$storage->flush();
		
		$query = synd_node_unit_test::getEntityQuery($storage);
		$query->addIdentifier($node1->nodeId);
		$query->addIdentifier($node2->nodeId);
		
		$collection = $query->getEntities();
		$this->assertEquals(2, count($collection));
		$this->assertEquals(2, $collection->count());
		
		$expected = array($node1->nodeId, $node2->nodeId);
		$actual = array_values(SyndLib::collect($collection->getIterator(0, null, array($query->getPrimaryKey())), 'nodeId'));
		$this->assertEquals($expected, $actual);

		$expected = array($node1->nodeId, $node2->nodeId);
		$actual = array_values(SyndLib::collect($collection->getIterator(0, 2, array($query->getPrimaryKey())), 'nodeId'));
		$this->assertEquals($expected, $actual);

		$expected = array($node1->nodeId);
		$actual = array_values(SyndLib::collect($collection->getIterator(0, 1, array($query->getPrimaryKey())), 'nodeId'));
		$this->assertEquals($expected, $actual);

		$expected = array($node2->nodeId);
		$actual = array_values(SyndLib::collect($collection->getIterator(1, 1, array($query->getPrimaryKey())), 'nodeId'));
		$this->assertEquals($expected, $actual);
		
		$node1->delete();
		$node2->delete();
	}
	
	function testAggregateCollection() {
		$storage = SyndNodeLib::getDefaultStorage('issue');
		$project = $storage->factory('project');
		$project->setTitle('_unit_test: This project is safe to delete');
		$project->save();
		
		$issue = $project->appendChild($storage->factory('issue'));
		$issue->setTitle('_unit_test: This issue is safe to delete');
		$issue->save();
		
		$note1 = $issue->appendChild($storage->factory('task'));
		$note1->data['CREATE_NODE_ID'] = 'user_null.a';
		$note1->save();

		$note2 = $issue->appendChild($storage->factory('task'));
		$note2->data['CREATE_NODE_ID'] = 'user_null.c';
		$note2->save();
		
		$storage->flush();
		$collection = $issue->getNotes();
		
		// Test iteration
		$actual = array_values(SyndLib::collect($collection->getIterator(0, null, array('CREATE_NODE_ID')), 'nodeId'));
		$this->assertEquals(array($note1->nodeId, $note2->nodeId), $actual);
		
		// Test append
		$note3 = $collection->append($storage->factory('task'));
		$note3->data['CREATE_NODE_ID'] = 'user_null.d';

		$actual = array_values(SyndLib::collect($collection->getIterator(0, null, array('CREATE_NODE_ID', false)), 'nodeId'));
		$this->assertEquals(array($note3->nodeId, $note2->nodeId, $note1->nodeId), $actual);
		
		// Test append
		$note4 = $collection->append($storage->factory('task'));
		$note4->data['CREATE_NODE_ID'] = 'user_null.b';

		$actual = array_values(SyndLib::collect($collection->getIterator(0, null, array('CREATE_NODE_ID')), 'nodeId'));
		$this->assertEquals(array($note1->nodeId, $note4->nodeId, $note2->nodeId, $note3->nodeId), $actual);

		$actual = array_values(SyndLib::collect($collection->getIterator(0, 2, array('CREATE_NODE_ID')), 'nodeId'));
		$this->assertEquals(array($note1->nodeId, $note4->nodeId), $actual);

		$actual = array_values(SyndLib::collect($collection->getIterator(1, 10, array('CREATE_NODE_ID')), 'nodeId'));
		$this->assertEquals(array($note4->nodeId, $note2->nodeId, $note3->nodeId), $actual);

		$actual = array_values(SyndLib::collect($collection->getIterator(1, 2, array('CREATE_NODE_ID')), 'nodeId'));
		$this->assertEquals(array($note4->nodeId, $note2->nodeId), $actual);

		$actual = array_values(SyndLib::collect($collection->getIterator(2, 2, array('CREATE_NODE_ID')), 'nodeId'));
		$this->assertEquals(array($note2->nodeId, $note3->nodeId), $actual);

		// Test remove
		$collection->remove($note1);
		$collection->remove($note4);

		$actual = array_values(SyndLib::collect($collection->getIterator(0, null, array('CREATE_NODE_ID')), 'nodeId'));
		$this->assertEquals(array($note2->nodeId, $note3->nodeId), $actual);
		
		// Test persistence
		$note3->save();
		$note3->flush();
		$collection->flush();
		
		$actual = array_values(SyndLib::collect($collection->getIterator(0, null, array('CREATE_NODE_ID', false)), 'nodeId'));
		$this->assertEquals(array($note3->nodeId, $note2->nodeId, $note1->nodeId), $actual);
		
		$database = $storage->getDatabase();
		$sql = "
			SELECT t.node_id FROM synd_issue_task t
			WHERE t.parent_node_id = ".$database->quote($issue->nodeId);
		$actual = $database->getCol($sql);
		natsort($actual);
		$this->assertEquals(array($note1->nodeId, $note2->nodeId, $note3->nodeId), $actual);
		
		// Test serialize
		$collection2 = unserialize(serialize($collection));
		$actual = array_values(SyndLib::collect($collection2->getIterator(0, null, array('CREATE_NODE_ID', false)), 'nodeId'));
		$this->assertEquals(array($note3->nodeId, $note2->nodeId, $note1->nodeId), $actual);

		$issue->delete();
		$project->delete();
	}
}
