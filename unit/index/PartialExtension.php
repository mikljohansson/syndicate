<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/IndexQuery.class.inc';
require_once 'core/index/SyndTermIndex.class.inc';
require_once 'core/index/SyndTextFilter.class.inc';
require_once 'core/index/SyndPartialExtension.class.inc';
require_once 'core/index/SyndPhraseExtension.class.inc';

class _index_PartialExtension extends SyndIndexTestCase {
	var $_index = null;

	function setUp() {
		$search = Module::getInstance('search');
		$this->_index = new SyndTermIndex($search->getIndexDatabase());
		$this->_index->loadExtension(new SyndTextFilter());
	}

	function testPartialQuery() {
		$this->_index->loadExtension(new SyndPartialExtension());

		$query = new IndexQuery('import');
		$builder = new SqlQueryBuilder($this->_index);
		$expression = $query->createExpression($this->_index);
		
		$expected = "td0.term LIKE 'import%'";
		$actual = $builder->parseExpression($expression->getBalanced());
		$this->assertEquals($expected, $actual);
		
//		$builder = new SqlQueryBuilder($this->_index);
//		$expression = new SyndNullQuery();
//		
//		$expected = "td0.term = ''";
//		$actual = $builder->parseExpression($expression);
//		$this->assertEquals($expected, $actual);
	}
	
	function testBranched() {
		$this->_index->loadExtension(new SyndPhraseExtension());
		
		$short = new SyndTermQuery('abc');
		$long = new SyndTermQuery('abcde');
		$phrase = new SyndPhraseQuery('abcde defgh');
		
		$visitor = new StatisticsExpressionVisitor($this->_index);
		$short->acceptVisitor($visitor);
		$this->assertEquals(1, $visitor->getBranches());

		$visitor = new StatisticsExpressionVisitor($this->_index);
		$long->acceptVisitor($visitor);
		$this->assertEquals(1, $visitor->getBranches());

		$visitor = new StatisticsExpressionVisitor($this->_index);
		$phrase->acceptVisitor($visitor);
		$this->assertEquals(1, $visitor->getBranches());

		$this->_index->loadExtension(new SyndPartialExtension());

		$visitor = new StatisticsExpressionVisitor($this->_index);
		$short->acceptVisitor($visitor);
		$this->assertEquals(1, $visitor->getBranches());

		$visitor = new StatisticsExpressionVisitor($this->_index);
		$long->acceptVisitor($visitor);
		$this->assertEquals(2, $visitor->getBranches());

		$visitor = new StatisticsExpressionVisitor($this->_index);
		$phrase->acceptVisitor($visitor);
		$this->assertEquals(3, $visitor->getBranches());
	}
	
	function testSearch() {
		$this->_index->loadExtension(new SyndPartialExtension());
		$this->clearSection($this->_index, '_unit_test');

		$this->_index->addDocument('_unit_test1',
			$this->_index->createFragment('unittestfoo'),
			'_unit_test');
		$this->_index->addDocument('_unit_test2',
			$this->_index->createFragment('unittes'),
			'_unit_test');
		$this->_index->addDocument('_unit_test3',
			$this->_index->createFragment('unittestbar'),
			'_unit_test');

		$this->_index->addDocument('_unit_test4',
			$this->_index->createFragment('unittestabcdefghijklmnopqrstuvxyz'),
			'_unit_test');

		$this->_index->flush();
		
		$mset = $this->_index->getMatchSet(new IndexQuery('unittest', array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		sort($mset);
		$this->assertEquals(array('_unit_test1','_unit_test3','_unit_test4'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery('unittestabcdefghijklmnopqrstuvxyz', array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test4'), $mset);
	}
}
