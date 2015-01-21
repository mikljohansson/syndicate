<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_Type_SearchCollection extends PHPUnit2_Framework_TestCase {
	function testSearch() {
		$id = md5(uniqid(''));
		$storage = SyndNodeLib::getDefaultStorage('unit_test');
		
		$node1 = $storage->factory('unit_test');
		$node1->data['INFO_HEAD'] = $id;
		$node1->save();
		
		$node2 = $storage->factory('unit_test');
		$node2->data['INFO_HEAD'] = $id.' foo';
		$node2->save();
		
		$storage->flush();
		
		$collection = SyndType::factory('search_collection', 'unit_test', $id);

		$mset = $collection->getContents(0, 10);
		$this->assertEquals(array($node1->id() => $node1, $node2->id() => $node2), SyndLib::sort($mset, 'nodeId'));
		$this->assertEquals(2, $collection->getCount());
		
		$this->assertEquals(2, $collection->_count);
		$collection = unserialize(serialize($collection));
		$this->assertNull($collection->_count);
		
		$collection->delete();
	}
}

