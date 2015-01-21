<?php
require_once 'unit/SyndNodeTestCase.class.inc';

class _model_DatabaseStorage extends SyndNodeTestCase {
	var $_errors = 0;
	
	function testInstance() {
		global $synd_maindb;
		$sql = "
			INSERT INTO synd_unit_test (node_id)
			VALUES ('unit_test.1')";
		$synd_maindb->query($sql);

		$sql = "
			INSERT INTO synd_unit_test2 (node_id, info_head)
			VALUES ('unit_test.1', 'Test')";
		$synd_maindb->query($sql);
		
		$storage = $this->getDatabaseStorage();
		$node = $storage->getInstance('unit_test.1');
		
		$this->assertEquals('unit_test.1', $node->data['NODE_ID']);
		$this->assertEquals('Test', $node->data['INFO_HEAD']);
		
		$nodes = $storage->getInstances(array('unit_test.1'));
		$this->assertEquals(1, count($nodes));
		
		if (!empty($nodes)) {
			$node = $nodes[key($nodes)];
			$this->assertEquals('unit_test.1', $node->data['NODE_ID']);
			$this->assertEquals('Test', $node->data['INFO_HEAD']);
		}
		
		// Test write to storage		
		$node->data['INFO_HEAD'] = 'Test2';
		$node->save();
		$storage->flush();
		
		$node = $storage->getInstance('unit_test.1');
		$this->assertEquals('Test2', $node->data['INFO_HEAD']);
		
		$sql = "
			SELECT t2.info_head FROM synd_unit_test2 t2
			WHERE t2.node_id = 'unit_test.1'";
		$this->assertEquals('Test2', $synd_maindb->getOne($sql));
		
		// Test delete from storage
		$node->delete();
		$storage->flush();

		$node = $storage->getInstance('unit_test.1');
		$this->assertNull($node);

		$sql = "
			SELECT t2.info_head FROM synd_unit_test2 t2
			WHERE t2.node_id = 'unit_test.1'";
		$this->assertNull($synd_maindb->getOne($sql));
	}

	function testFactory() {
		global $synd_maindb;
		$storage = $this->getDatabaseStorage();
		$node = $storage->factory('unit_test');
		
		$this->assertTrue($node->isNew());
		$this->assertEquals('Test', $node->data['INFO_HEAD']);
		
		$node->save();
		$storage->flush();

		$this->assertFalse($node->isNew());
		$sql = "
			SELECT t.node_id FROM synd_unit_test t
			WHERE t.node_id = ".$synd_maindb->quote($node->nodeId);
		
		$this->assertNotNull($node->nodeId);
		$this->assertEquals($node->nodeId, $synd_maindb->getOne($sql));
		
		$node2 = $storage->getInstance($node->nodeId);
		$this->assertNotNull($node2);
		$this->assertEquals($node->nodeId, $node2->nodeId);
	}
	
	function testAutoIncrement() {
		$node = SyndNodeLib::factory('unit_test');
		$this->assertNull($node->data['SEQUENCE_ID']);
		
		$node->save();
		$node->flush();
		
		$this->assertNotNull($node->data['SEQUENCE_ID']);
	}
	
	function testBatchSize() {
		$storage = $this->getDatabaseStorage();
		$node = $storage->factory('unit_test');
		$node->save();
		$node->flush();
		
		$ids = array($node->nodeId);
		for ($i=0, $oid=$node->objectId(); $i<1500; $i++)
			$storage->preload('unit_test.'.($oid+$i));
		
		$node2 = $storage->getInstance($node->nodeId);
		$this->assertEquals($node->nodeId, $node2->nodeId);
	
	}

	function testReferences() {
		$storage = $this->getDatabaseStorage();
		$node = $storage->factory('unit_test');
		$node->data['INFO_HEAD'] = 'Test';
		
		$node->save();
		$node->flush();
		
		$data = $node->data;
		$head = $node->data['INFO_HEAD'];
		
		$data['INFO_HEAD'] = 'Test2';
		$this->assertEquals($head, $node->data['INFO_HEAD']);
	}
	
	function testDatabaseCollection() {
		$storage = $this->getDatabaseStorage();
		$node1 = $storage->factory('unit_test');
		$node2 = clone $node1;
		
		$node1->save();
		$node2->save();
		$storage->flush();
		
		$query = synd_node_unit_test::getEntityQuery($storage);
		$query->addIdentifier($node1->nodeId);
		$query->addIdentifier($node2->nodeId);
		$query->order($query->getPrimaryKey());
		
		$collection = new DatabaseEntityCollection($storage, $query);
		$expected = array($node1->nodeId, $node2->nodeId);
		
		$actual = count($collection);
		$this->assertEquals(count($expected), $actual);
		
		$actual = array_values(SyndLib::collect(iterator_to_array($collection->getIterator()), 'nodeId'));
		$this->assertEquals($expected, $actual);
		
		// Test limit iterator
		$iter = $collection->getIterator(1, 250);
		$expected = array($node2->nodeId);
		$actual = array_values(SyndLib::collect(iterator_to_array($iter), 'nodeId'));
		$this->assertEquals($expected, $actual);
		
		$node1->delete();
		$node2->delete();
	}
}
