<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _model_Type_Text extends PHPUnit2_Framework_TestCase {
	function testCallback() {
		$expected = 'The test text';
		$text = SyndType::factory('text', $expected);
		$this->assertEquals($expected, $text->toString());
		$this->assertEquals($expected, trim(strip_tags(tpl_gui_node($text,'full_view.tpl'))));
		$this->assertEquals("$expected. Something else", trim(strip_tags(tpl_gui_node($text,'full_view.tpl',array('filter' => array($this, '_callback_filter'))))));
	}

	function _callback_filter($text) {
		return "$text. Something else";
	}
}