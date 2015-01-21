<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_Node_Folder extends PHPUnit2_Framework_TestCase {
	function testInheritedClasses() {
		// Test exclude inherited accepted classes
		$folder = SyndNodeLib::factory('folder');
		$folder->setAcceptedClasses(array('item','computer'));
		
		$this->assertFalse($folder->isInheritedClass('item'));
		
		$expected = array('item');
		$actual = $folder->getClasses();
		$this->assertEquals($expected, array_values($actual));
		
		// Test inherit classes from parent folder
		$parent = SyndNodeLib::factory('folder');
		$parent->setAcceptedClasses(array('item'));
		
		$folder2 = $parent->appendChild(SyndNodeLib::factory('folder'));
		$folder2->setAcceptedClasses(array('computer'));

		$this->assertFalse($parent->isInheritedClass('item'));
		$this->assertTrue($folder2->isInheritedClass('item'));
		$this->assertTrue($folder2->isInheritedClass('computer'));
		
		$actual = $folder2->getClasses();
		$this->assertEquals($expected, array_values($actual));
	}
}
