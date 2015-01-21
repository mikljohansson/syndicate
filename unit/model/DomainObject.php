<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_DomainObject extends PHPUnit2_Framework_TestCase {
	function testCompare() {
		$node1 = SyndNodeLib::factory('unit_test');
		$node2 = SyndNodeLib::factory('unit_test');
		$node3 = SyndNodeLib::factory('unit_test');

		$node1->data['INFO_DESC'] = 'Desc1';
		$node2->data['INFO_DESC'] = 'Desc1';
		$node3->data['INFO_DESC'] = 'desc2';
		
		$node1->data['INFO_HEAD'] = 'Test1';
		$node2->data['INFO_HEAD'] = 'test2';
		$node3->data['INFO_HEAD'] = 'Test3';
		
		$order = array('INFO_DESC', 'INFO_HEAD');
		
		$this->assertEquals(0, $node1->compare($node1, $order));
		$this->assertEquals(-1, $node1->compare($node2, $order));
		$this->assertEquals(-1, $node1->compare($node3, $order));

		$this->assertEquals(1, $node2->compare($node1, $order));
		$this->assertEquals(-1, $node2->compare($node3, $order));

		$this->assertEquals(1, $node3->compare($node1, $order));
		$this->assertEquals(1, $node3->compare($node2, $order));

		$order = array('INFO_DESC', false, 'INFO_HEAD');
		
		$this->assertEquals(0, $node1->compare($node1, $order));
		$this->assertEquals(-1, $node1->compare($node2, $order));
		$this->assertEquals(1, $node1->compare($node3, $order));

		$this->assertEquals(1, $node2->compare($node1, $order));
		$this->assertEquals(1, $node2->compare($node3, $order));

		$this->assertEquals(-1, $node3->compare($node1, $order));
		$this->assertEquals(-1, $node3->compare($node2, $order));
	}

	function testAttributes() {
		$node = SyndNodeLib::factory('unit_test');
		$node->setAttribute('_unit_test', 'Test');
		$actual = $node->getAttribute('_unit_test');
		$this->assertEquals('Test', $actual);
		
		$node2 = $node->appendChild($node->_storage->factory('unit_test'));
		$actual = $node2->getAttribute('_unit_test');
		$this->assertEquals('Test', $actual);
		
		$type = $node2->appendChild(SyndType::factory('text'));
		$actual = $type->getAttribute('_unit_test');
		$this->assertEquals('Test', $actual);
	}
}
