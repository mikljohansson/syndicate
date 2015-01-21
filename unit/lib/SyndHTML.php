<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/lib/SyndHTML.class.inc';

class _lib_SyndHTML extends PHPUnit2_Framework_TestCase {
	function testFilterHtml() {
		$text = 'A target <a href="http://www.example.com" target="_blank" onclick="evilFunction();">link</a> <span target="_blank">invalid span</span>';
		$expected = 'A target <a href="http://www.example.com" target="_blank">link</a> <span>invalid span</span>';
		$actual = SyndHTML::filterHtml($text);
		$this->assertEquals($expected, $actual);

		$text = 'A invalid target <a href="http://www.example.com" target="_foo">link</a>';
		$expected = 'A invalid target <a href="http://www.example.com" target="_self">link</a>';
		$actual = SyndHTML::filterHtml($text);
		$this->assertEquals($expected, $actual);
	}
}
