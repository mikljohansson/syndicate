<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/SyndExpression.class.inc';

class _index_Expression extends SyndIndexTestCase {
	function testHeight() {
		$expression = new SyndTermQuery('foo');
		$this->assertEquals(1, $expression->getHeight());
		
		$expression = new SyndBooleanAND(
			new SyndTermQuery('foo'),
			new SyndTermQuery('bar'));
		$this->assertEquals(2, $expression->getHeight());
	
		$expression = new SyndBooleanAND(
			new SyndTermQuery('foo'),
			new SyndBooleanAND(
				new SyndTermQuery('bar'),
				new SyndTermQuery('foobar')));
		$this->assertEquals(3, $expression->getHeight());

		$expression = new SyndBooleanOR(
			new SyndTermQuery('foo'),
			new SyndTermQuery('bar'));
		$this->assertEquals(1, $expression->getHeight());

		$expression = new SyndBooleanOR(
			new SyndTermQuery('foo'),
			new SyndBooleanAND(
				new SyndTermQuery('bar'),
				new SyndTermQuery('foobar')));
		$this->assertEquals(2, $expression->getHeight());

		$expression = new SyndBooleanInclude(
			new SyndBooleanAND(
				new SyndTermQuery('foo'),
				new SyndTermQuery('foobar')),
			new SyndTermQuery('bar'));
		$this->assertEquals(3, $expression->getHeight());

		$expression = new SyndBooleanExclude(
			new SyndBooleanAND(
				new SyndTermQuery('foo'),
				new SyndTermQuery('foobar')),
			new SyndTermQuery('bar'));
		$this->assertEquals(2, $expression->getHeight());
	}
	
	function testBounded() {
		$expression = new SyndTermQuery('foo');
		$this->assertTrue($expression->isBounded());

		$expression = new SyndBooleanAND(
			new SyndTermQuery('foo'),
			new SyndTermQuery('bar'));
		$this->assertTrue($expression->isBounded());

		$expression = new SyndBooleanInclude(
			null,
			new SyndTermQuery('bar'));
		$this->assertTrue($expression->isBounded());

		$expression = new SyndBooleanExclude(
			null,
			new SyndTermQuery('bar'));
		$this->assertFalse($expression->isBounded());

		$expression = new SyndBooleanInclude(
			new SyndTermQuery('bar'),
			null);
		$this->assertTrue($expression->isBounded());

		$expression = new SyndBooleanExclude(
			new SyndTermQuery('bar'),
			null);
		$this->assertTrue($expression->isBounded());
	}
}

?>