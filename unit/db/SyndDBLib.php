<?php
require_once 'PHPUnit2/Framework/TestCase.php';
require_once 'core/db/SyndDBLib.class.inc';

class _db_SyndDBLib extends PHPUnit2_Framework_TestCase {
	function testQuote() {
		$this->assertEquals("'foo''bar\\'", SyndDBLib::quote("foo'bar\\"));
		$this->assertEquals("'foo''bar'", SyndDBLib::quote("foo'bar"));
	}
	
	function testSqlWhere() {
		$arr = array('foo' => "b'ar", 'foo2' => null);
		$ret = SyndDBLib::sqlWhere($arr);
		$this->assertEquals("foo = 'b''ar' AND foo2 = NULL", $ret);
	}

	function testSqlUpdate() {
		$arr = array('foo' => "b'ar", 'foo2' => null);
		$ret = SyndDBLib::sqlUpdate($arr);
		$this->assertEquals("foo = 'b''ar', foo2 = NULL", $ret);
	}

	function testSqlCols() {
		$arr = array('foo' => "b'ar", 'foo2' => null);
		$ret = SyndDBLib::sqlCols($arr);
		$this->assertEquals("foo, foo2", $ret);
	}

	function testSqlValues() {
		$arr = array('foo' => "b'ar", 'foo2' => null);
		$ret = SyndDBLib::sqlValues($arr);
		$this->assertEquals("'b''ar', NULL", $ret);
	}

	function testParseSearchString() {
		$str = 'foo -bar "foo bar" " + blah blah" -- "bl bah  ';
		$arr = array('foo', '-bar', 'foo bar', '+blah blah', '-bl bah');
		$ret = SyndDBLib::parseSearchString($str);
		$this->assertEquals($arr, $ret);
	}
}
