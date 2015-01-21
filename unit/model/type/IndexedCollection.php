<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/index/SyndTermIndex.class.inc';
require_once 'core/index/SyndTextFilter.class.inc';
require_once 'core/index/IndexQuery.class.inc';

class _model_Type_IndexedCollection extends PHPUnit2_Framework_TestCase {
	var $_index = null;

	function setUp() {
		$search = Module::getInstance('search');
		$this->_index = new SyndTermIndex($search->getIndexDatabase());
		$this->_index->loadExtension(new SyndTextFilter());
	}

	function testSearch() {
		$this->_index->clearSection('n.unit_test');
		$id = '_unit_test_'.md5(uniqid(''));

		$storage = SyndNodeLib::getDefaultStorage('unit_test');
		$node1 = $storage->factory('unit_test');
		$node1->save();
		$node2 = $storage->factory('unit_test');
		$node2->save();
		$storage->flush();

		$this->_index->addDocument($node1->nodeId, $this->_index->createFragment($id), 'n.unit_test');
		$this->_index->addDocument($node2->nodeId, $this->_index->createFragment($id), 'n.unit_test');
		$this->_index->commit();

		$collection = SyndType::factory('indexed_collection', new IndexQuery($id, array('n.unit_test')), $this->_index);

		$mset = $collection->getContents(0, 1);
		$this->assertEquals(array($node1->id() => $node1), $mset);
		$this->assertEquals(2, $collection->getCount());

		$mset = $collection->getContents(0, 10);
		$this->assertEquals(array($node1->id() => $node1, $node2->id() => $node2), SyndLib::sort($mset, 'nodeId'));
		$this->assertEquals(2, $collection->getCount());
		
		$this->assertEquals(2, $collection->_count);
		$collection = unserialize(serialize($collection));
		$this->assertNull($collection->_count);

		$mset = $collection->getContents(1, 2);
		$this->assertEquals(array($node2->id() => $node2), $mset);
		$this->assertEquals(2, $collection->getCount());
		
		$mset = $collection->getFilteredContents(array('synd_node_unit_test'));
		$this->assertEquals(array($node1->id() => $node1, $node2->id() => $node2), $mset);
		$this->assertEquals(2, $collection->getFilteredCount(array('synd_node_unit_test')));

		$mset = $collection->getFilteredContents(array('synd_node_case'));
		$this->assertEquals(array(), $mset);
		$this->assertEquals(0, $collection->getFilteredCount(array('synd_node_case')));
		
		$collection->delete();
	}
}
