<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_Type_OleCollection extends PHPUnit2_Framework_TestCase {
	function testInitialize() {
		$node1 = SyndNodeLib::factory('unit_test');
		$node1->data['OBJECT_ID'] = 1;
		$node1->data['SEQUENCE_ID'] = 1;
		$node1->save();
		$node1->flush();
		
		$node2 = SyndNodeLib::factory('unit_test');
		$node2->data['OBJECT_ID'] = 2;
		$node2->data['SEQUENCE_ID'] = 2;
		$node2->save();
		$node2->flush();

		$_REQUEST['selection'] = array($node1->id(), $node2->id());
		$collection = SyndLib::getInstance('type.ole_collection.selection');
		
		$contents = $collection->getContents();
		$this->assertEquals(array($node1->id() => $node1, $node2->id() => $node2), $contents);
		$this->assertEquals(2, $collection->getCount());

		$collection->delete();
	}
}

?>