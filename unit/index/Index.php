<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/AbstractIndex.class.inc';

class _index_Index extends SyndIndexTestCase {
	var $_i = 0;
	var $_last = 0;
	
	function _callback_extension(&$result, &$ref) {
		$result = $ref;
		$this->_i++;
		$ref++;
		$this->_last = 1;
	}
	
	function _callback_extension2(&$result, &$ref) {
		$this->_last = 2;
	}
	
	function testExtensionHooks() {
		$ref = 10;
		$lasti = $this->_i;
		
		$index = new AbstractIndex();
		$index->attachHook('_unit_test', array($this, '_callback_extension'));
		$index->attachHook('_unit_test', array($this, '_callback_extension2'));
		$i = $index->runHook('_unit_test', array(&$ref));
		
		$this->assertEquals($lasti+1, $this->_i);
		$this->assertEquals(11, $ref);
		$this->assertEquals(10, $i);
		$this->assertEquals(2, $this->_last);
	}
}