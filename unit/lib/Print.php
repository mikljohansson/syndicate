<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/lib/SyndPrint.class.inc';

class _lib_Print extends PHPUnit2_Framework_TestCase {
	function testMultiLine() {
		$buffer = file_get_contents(dirname(__FILE__).'/_print/print-multiline.ps');
		
		$expected = str_replace('(FOO_PAD)', '(1234567)', $buffer);
		$expected = str_replace('(FOO_PADPAD)', '(8901234567)', $expected);
		$expected = str_replace('(FOO_PADPADPAD)', '(890123)', $expected);
		$expected = str_replace('(BAR_______)', '(Bar)', $expected);
		
		$this->assertNotSame(false, strpos($expected, '(1234567)'));
		$this->assertNotSame(false, strpos($expected, '(8901234567)'));
		$this->assertNotSame(false, strpos($expected, '(890123)'));
		$this->assertNotSame(false, strpos($expected, '(Bar)'));
		
		$filter = array(
			'FOO' => '12345678901234567890123',
			'BAR' => 'Bar');
		$actual = SyndPrint::psFilter($buffer, $filter);
		
		$this->assertEquals($expected, $actual);
	}
	
	function testEscapePDF() {
		$value = "едц ()\\";
		$expected = "(\\345\\344\\366 \\(\\)\\\\)";
		$actual = SyndPrint::pdfEscape($value);
		$this->assertEquals($expected, $actual);
	}

	function testEscapePDFName() {
		$value = "едц ()\\#";
		$expected = "/#e5#e4#f6#20()\\#23";
		$actual = SyndPrint::pdfEscapeName($value);
		$this->assertEquals($expected, $actual);
	}
	
	function testGenerateFDF() {
		$expected = file_get_contents(dirname(__FILE__).'/_print/print-001.fdf');
		
		$uri = 'http://www.example.com/example.pdf';
		$values = array(
			'Field1' => '123',
			'Field2' => '456',
			);
		$names = array(
			'Checkbox1' => true,
			'Radiobutton2' => false,
			);
		
		$actual = SyndPrint::fdfGenerate($values, $names, $uri);
		$this->assertEquals($expected, $actual);
	}
	
	function testMergePDF() {
		$expected = file_get_contents(dirname(__FILE__).'/_print/print-002-output.pdf');
		$filter = array(
			'Field1' => '123',
			'Field2' => '456',
			);
		
		$actual = SyndPrint::pdfMerge(dirname(__FILE__).'/_print/print-002-input.pdf', $filter);
		$this->assertNotNull($actual);

		// Output is slighly different from system to system
		// $this->assertEquals($expected, $actual);
	}
}
