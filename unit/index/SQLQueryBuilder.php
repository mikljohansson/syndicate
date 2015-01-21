<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/IndexQuery.class.inc';
require_once 'core/index/SyndTermIndex.class.inc';
require_once 'core/index/SyndTextFilter.class.inc';
require_once 'core/index/SyndPhraseExtension.class.inc';

class _index_SQLQueryBuilder extends SyndIndexTestCase {
	var $_index = null;

	function setUp() {
		$search = Module::getInstance('search');
		$this->_index = new SyndTermIndex($search->getIndexDatabase());
		$this->_index->loadExtension(new SyndTextFilter());
	}

	function testBasic() {
		$str = "regulation || import && export";

		$expected = "((t0.termid = 1408025241 AND t1.termid = 0) OR (t0.termid = -1655779811 AND t1.termid = 1116477076))";
		$query = new IndexQuery($str);
		$expr = $query->createExpression($this->_index);

		$builder = new SqlQueryBuilder($this->_index);
		$actual = $builder->parseExpression($expr->getBalanced());

		$this->assertEquals($expected, $actual);
	}

	function testTableReuse() {
		$str = "regulation || import && export || foo";

		$expected = "(((t0.termid = 1408025241 AND t1.termid = 0) OR (t0.termid = -1655779811 AND t1.termid = 1116477076)) OR (t0.termid = -1938594527 AND t1.termid = 0))";
		$query = new IndexQuery($str);
		$expr = $query->createExpression($this->_index);
		
		$builder = new SqlQueryBuilder($this->_index);
		$actual = $builder->parseExpression($expr->getBalanced());

		$this->assertEquals($expected, $actual);
	}
	
	function testFrom() {
		$str = "regulation || import && export || foo";

		$expected = "synd_search_termindex t0, synd_search_termindex t1";

		$query = new IndexQuery($str);
		$expr = $query->createExpression($this->_index);
		
		$builder = new SqlQueryBuilder($this->_index);
		$builder->parseExpression($expr->getBalanced());
		$actual = $builder->getFrom();
		
		$this->assertEquals($expected, $actual);
	}
}

?>