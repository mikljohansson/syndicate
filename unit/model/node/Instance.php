<?php
require_once 'unit/SyndNodeTestCase.class.inc';

class _model_Node_Instance extends SyndNodeTestCase {
//	function testFields() {
//		$node = SyndNodeLib::factory('instance');
//		$class = $node->_storage->factory('class');
//		$class->setTitle('_unit_test: This class is safe to delete');
//
//		$names = array('Test','Test2', 'Test3');
//		$class->addField('Test', 'string');
//		$class->addField('Test2', 'string');
//		$class->addField('Test3', 'mac');
//		
//		$fields = SyndLib::sort((array)$class->getFields());
//		$this->assertEquals($names, array_values(SyndLib::invoke($fields, 'toString')));
//		
//		$node->setClass($class);
//		$ids = array_keys($fields);
//		$node->merge(array('values' => array(
//			array_shift($ids) => 'test',
//			array_shift($ids) => 'test2')));
//		
//		$class->save();
//		$node->save();
//		$node->flush();
//		
//		// Reload node from storage
//		$persistent = $node->_storage->getPersistentStorage();
//		$node2 = $persistent->getInstance($node->nodeId);
//		
//		$values = $node->getValues();
//		$values2 = $node2->getValues();
//		$this->assertTrue(in_array('test', $values));
//		$this->assertTrue(in_array('test', $values2));
//		$this->assertTrue(in_array('test2', $values));
//		$this->assertTrue(in_array('test2', $values2));
//
//		// Test merging composite values
//		$keys = array_keys($fields);
//		$node->merge(array('value' => array(reset($keys),'somevalue')));
//		$values = $node->getValues();
//		$this->assertEquals('somevalue', $values[reset($keys)]);
//		
//		// Test modify values in class before store
//		$data = array('values' => array(end($keys) => '01020304abdc'));
//		$errors = $node2->validate($data);
//		$this->assertTrue(empty($errors));
//		$node2->merge($data);
//		$values = $node2->getValues();
//		$this->assertEquals('01:02:03:04:AB:DC', $values[end($keys)]);
//		
//		// Reload class from storage
//		$class2 = $persistent->getInstance($class->nodeId);
//		$this->assertEquals($names, array_values(SyndLib::invoke((array)$class->getFields(),'toString')));
//		$this->assertEquals($names, array_values(SyndLib::invoke((array)$class2->getFields(),'toString')));
//		
//		$node->delete();
//		$class->delete();
//		
//		// Test merging CLASS_NODE_ID
//		$node = SyndNodeLib::factory('instance');
//		$class = $node->_storage->factory('class');
//		$class->addField('Test', 'string');
//		$class->setTitle('_unit_test: This class is safe to delete');
//		
//		$node->merge(array('CLASS_NODE_ID' => $class->nodeId));
//		$class2 = $node->getClass();
//		$this->assertNotNull($class->nodeId);
//		$this->assertEquals($class->nodeId, $class2->nodeId);
//	}

	function testInheritedValues() {
		$class = SyndNodeLib::factory('class');
		$class->addField('tftp-server-name', 'string');
		$fields = $class->getFields();
		$keys = array_keys($fields);
		
		$value = 'tftp.example.com';
		$node = SyndNodeLib::factory('instance');
		$node->setClass($class);
		$node->merge(array('values' => array(reset($keys) => $value)));

		$data = $node->getOptionalValues();
		$this->assertEquals($value, $data[reset($keys)]);
		
		$actual = array_values($node->getOptionalDefinitions());
		$expected = array(synd_node_class::type('string','tftp-server-name'));
		$this->assertEquals($expected, $actual);
		
		// Test inheritance of values and definitions
		$node2 = $node->appendChild(SyndNodeLib::factory('instance'));
		$data = $node2->getOptionalValues();
		$this->assertEquals($value, $data[reset($keys)]);

		$actual = array_values($node2->getOptionalDefinitions());
		$expected = array(synd_node_class::type('string','tftp-server-name'));
		$this->assertEquals($expected, $actual);
	}
}