<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/IndexQuery.class.inc';
require_once 'core/index/SyndTermIndex.class.inc';
require_once 'core/index/SyndTextFilter.class.inc';

class _index_Query extends SyndIndexTestCase {
	var $_index = null;

	function setUp() {
		$this->_index = new SyndTermIndex($GLOBALS['synd_maindb']);
		$this->_index->loadExtension(new SyndTextFilter());
	}

	function testIncludeQuery() {
		$str = "regulation +import foo +export";
		
		$expected = 
			new SyndBooleanInclude(
				new SyndBooleanOR(
					new SyndBooleanOR(
						new SyndTermQuery('regulation'),
						new SyndTermQuery('foo')
						),
					new SyndNullQuery()
					),
				new SyndBooleanAND(
					new SyndTermQuery('import'),
					new SyndTermQuery('export')
					)
				);
				
		$query = new IndexQuery($str);
		$actual = $query->createExpression($this->_index);
		
		$this->assertEquals($expected, $actual);
	}
	
	function testBasicBoolean() {
		$str = "regulation || import && export";

		$expected = 
			new SyndBooleanOR(
					new SyndTermQuery('regulation'),
					new SyndBooleanAND(
						new SyndTermQuery('import'),
						new SyndTermQuery('export')
					)
				);
		
		$query = new IndexQuery($str);
		$actual = $query->createExpression($this->_index);
		$this->assertEquals($expected, $actual);
	}

	function testDefaultOperator() {
		// Test defaulting OR
		$str = "regulation import && export";
		
		$expected = 
			new SyndBooleanOR(
					new SyndTermQuery('regulation'),
					new SyndBooleanAND(
						new SyndTermQuery('import'),
						new SyndTermQuery('export')
					)
				);
		$query = new IndexQuery($str);
		$actual = $query->createExpression($this->_index);
		$this->assertEquals($expected, $actual, '1');

		// Test defaulting AND
		$expected = 
			new SyndBooleanAND(
					new SyndBooleanAND(
						new SyndTermQuery('regulation'),
						new SyndTermQuery('import')),
					new SyndTermQuery('export')
				);
		$query = new IndexQuery($str, null, null, '&&');
		$actual = $query->createExpression($this->_index);
		$this->assertEquals($expected, $actual, '2');
		
		// Test defaulting on last term
		$expected =&
			new SyndBooleanOR(
					new SyndBooleanOR(
						new SyndTermQuery('regulation'),
						new SyndTermQuery('import')),
					new SyndTermQuery('export')
				);
		$query = new IndexQuery('regulation || import export');
		$actual = $query->createExpression($this->_index);
		
		$this->assertEquals($expected, $actual, '3');
	}
	
	function testStripQueryText() {
		$expected = "%& regulation ace || <br> <br /> 'a import && aef ap export";
		$actual = $this->_index->stripQueryText("%& regulation ÃÇÈ || <br> <br /> 'a import && Æf ap export");
		$this->assertEquals($expected, $actual);
	}

	function testBalanceAND() {
		$tree = new SyndBooleanOR(
			new SyndBooleanAND(
				new SyndTermQuery('t1'),
				new SyndTermQuery('t2')
				),
			new SyndBooleanAND(
				new SyndBooleanAND(
					new SyndTermQuery('t1'),
					new SyndTermQuery('t2')
					),
				new SyndTermQuery('t3')
				)
			);
		
		$expected = new SyndBooleanOR(
			new SyndBooleanAND(
				new SyndTermQuery('t1'),
				new SyndBooleanAND(
					new SyndTermQuery('t2'),
					new SyndNullQuery()
					)
				),
			new SyndBooleanAND(
				new SyndBooleanAND(
					new SyndTermQuery('t1'),
					new SyndTermQuery('t2')
					),
				new SyndTermQuery('t3')
				)
			);
		
		$actual = $tree->getBalanced();
		$this->assertEquals($expected, $actual);
	}
	
	function testExclude() {
		$query = new IndexQuery('foo-bar -bar');
		$expected = new SyndBooleanExclude(
			new SyndBooleanOR(
				new SyndTermQuery('foo'),
				new SyndTermQuery('bar')),
			new SyndTermQuery('bar'));
		$actual = $query->createExpression($this->_index);
		$this->assertEquals($expected, $actual);

		$query = new IndexQuery('-foo-bar');
		$expected = new SyndBooleanExclude(
			new SyndTermQuery('bar'),
			new SyndTermQuery('foo'));
		$actual = $query->createExpression($this->_index);
		
		$this->assertEquals($expected, $actual);
	}
}

?>