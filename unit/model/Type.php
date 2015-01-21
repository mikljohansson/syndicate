<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_Type extends PHPUnit2_Framework_TestCase {
	function testUnserializeReference() {
		$orig = new _ModelTypeUnittest();

		$b = unserialize(serialize($orig));
		$b->foo = 'bar';
		
		$this->assertEquals('bar', $b->foo);
		$this->assertEquals($b->foo, $GLOBALS['__unittest']->foo);
	}
}

class _ModelTypeUnittest {
	function __wakeup() {
		unset($GLOBALS['__unittest']);
		$GLOBALS['__unittest'] = $this;
	}
}
