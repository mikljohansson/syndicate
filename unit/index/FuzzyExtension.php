<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/SyndTermIndex.class.inc';
require_once 'core/index/SyndExpression.class.inc';
require_once 'core/index/SyndTextFilter.class.inc';
require_once 'core/index/SyndFuzzyExtension.class.inc';
require_once 'core/index/IndexQuery.class.inc';

class _index_FuzzyExtension extends SyndIndexTestCase {
	var $_index = null;

	function setUp() {
		$search = Module::getInstance('search');
		$this->_index = new SyndTermIndex($search->getIndexDatabase());
	}

	function testStripQueryText() {
		$expected = "foo bar~";
		$this->_index->loadExtension(new SyndTextFilter());
		$this->_index->loadExtension(new SyndFuzzyExtension());
		$this->assertEquals($expected, $this->_index->stripQueryText($expected));
	}

	function testGetExpression() {
		$actual = $this->_index->createExpression('~');
		$this->assertFalse(($actual instanceof SyndFuzzyOperator));

		$this->_index->loadExtension(new SyndFuzzyExtension());
		$actual = $this->_index->createExpression('~');
		$this->assertTrue(($actual instanceof SyndFuzzyOperator));
	}
	
	function testParse() {
		$str = "foo bar~";
		$this->_index->loadExtension($extension = new SyndFuzzyExtension());
		
		$expected = new SyndBooleanOR(
			new SyndTermQuery('foo'),
			new SyndFuzzyOperator($extension,
				new SyndTermQuery('bar')
				)
			);
		
		$query = new IndexQuery($str);
		$actual = $query->createExpression($this->_index);
		
		$this->assertEquals($expected, $actual);
	}

	function testBounded() {
		$extension = new SyndFuzzyExtension();
		$expression = new SyndFuzzyOperator($extension, new SyndTermQuery('bar'));
		$this->assertTrue($expression->isBounded());

		$expression = new SyndFuzzyOperator($extension, new SyndBooleanOR());
		$this->assertFalse($expression->isBounded());
	}

	function testBuild() {
		$this->_index->loadExtension(new SyndFuzzyExtension('metaphone'));
		$str = "foo bar~";
		
		$sound = metaphone('bar');
		$expected = "(t0.termid = -1938594527 OR (td0.term LIKE 'bar%' OR td0.fuzzy = '$sound'))";

		$query = new IndexQuery($str);
		$queryTree = $query->createExpression($this->_index);

		$builder = new SqlQueryBuilder($this->_index);
		$actual = $builder->parseExpression($queryTree);
		$this->assertEquals($expected, $actual);
	}

	function testDoubleMetaphone() {
		$this->_index->loadExtension($extension = new SyndFuzzyExtension('double_metaphone'));
		$str = 'foobar';
		
		$extension->enterFuzzyBranch();
		$extension->_callback_query_visit_term($actual, new SqlQueryBuilder($this->_index), new SyndTermQuery($str));
		
		$sound = double_metaphone($str);
		$expected = "(td0.term LIKE '$str%' OR td0.fuzzy = '$sound')";
		
		$this->assertEquals($expected, $actual);
	}
	
	function testSearch() {
		$this->_index->loadExtension(new SyndFuzzyExtension('metaphone'));
		$this->clearSection($this->_index, '_unit_test');

		$this->_index->addDocument('_unit_test1',
			$this->_index->createFragment('foobar'),
			'_unit_test');
		$this->_index->addDocument('_unit_test2',
			$this->_index->createFragment('foobr'),
			'_unit_test');
		$this->_index->addDocument('_unit_test3',
			$this->_index->createFragment('foob'),
			'_unit_test');
		$this->_index->addDocument('_unit_test4',
			$this->_index->createFragment('foobare'),
			'_unit_test');

		$this->_index->flush();
		
		$mset = $this->_index->getMatchSet(new IndexQuery('foobr~', array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		sort($mset);
		$this->assertEquals(array('_unit_test1','_unit_test2','_unit_test4'), $mset);
	}
}

?>