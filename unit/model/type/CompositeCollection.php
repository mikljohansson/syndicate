<?php
require_once 'unit/SyndNodeTestCase.class.inc';

class _model_Type_CompositeCollection extends SyndNodeTestCase {
	function testList() {
		$storage = SyndNodeLib::getDefaultStorage('unit_test');
		$node1 = $storage->factory('unit_test');
		$node1->save();
		$node2 = $storage->factory('unit_test');
		$node2->save();
		$node3 = $storage->factory('unit_test');
		$node3->save();
		$node4 = $storage->factory('unit_test');
		$node4->save();
		$storage->flush();
		
		$alias = DatabaseConnection::alias('synd_unit_test');
		$collection = SyndType::factory('composite_collection', array(
			SyndType::factory('ole_collection', 'unit_test', array($node1->id())),
			SyndType::factory('ole_collection', 'unit_test', array($node2->id(), $node3->id())),
			SyndType::factory('ole_collection', 'unit_test', array($node4->id())),
			));

		$mset = $collection->getContents(0, 1);
		$this->assertEquals(array($node1->id() => $node1), $mset);
		
		$mset = $collection->getContents(1, 2);
		$this->assertEquals(array($node2->id() => $node2, $node3->id() => $node3), $mset);

		$mset = $collection->getContents(2, 2);
		$this->assertEquals(array($node3->id() => $node3, $node4->id() => $node4), $mset);

		$mset = $collection->getContents(3);
		$this->assertEquals(array($node4->id() => $node4), $mset);

		$mset = $collection->getFilteredContents(array('synd_node_unit_test'));
		$this->assertEquals(array(
			$node1->id() => $node1, 
			$node2->id() => $node2, 
			$node3->id() => $node3, 
			$node4->id() => $node4), $mset);
		$this->assertEquals(4, $collection->getFilteredCount(array('synd_node_unit_test')));

		$mset = $collection->getFilteredContents(array('synd_node_case'));
		$this->assertEquals(array(), $mset);
		$this->assertEquals(0, $collection->getFilteredCount(array('synd_node_case')));
		
		$collection->delete();
	}
}
