<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/IndexQuery.class.inc';
require_once 'core/index/SyndTermIndex.class.inc';
require_once 'core/index/SyndNullIndex.class.inc';
require_once 'core/index/SyndTextFilter.class.inc';
require_once 'core/index/SyndPhraseExtension.class.inc';
require_once 'core/index/BM25WeightingScheme.class.inc';

class _index_PhraseExtension extends SyndIndexTestCase {
	var $_index = null;

	function setUp() {
		$search = Module::getInstance('search');
		$this->_index = new SyndTermIndex($search->getIndexDatabase());
		$this->_index->loadExtension(new SyndTextFilter());
		$this->_index->loadExtension(new BM25WeightingScheme());
	}

	function testLexerExpression() {
		$this->_index->loadExtension(new SyndPhraseExtension());
		$this->assertTrue(in_array('"',$this->_index->lexerExpression()));
	}

	function testExpression() {
		$actual = $this->_index->createExpression('"');
		$this->assertFalse(($actual instanceof SyndPhraseQuery));

		$this->_index->loadExtension(new SyndPhraseExtension());

		$actual = $this->_index->createExpression('"');
		$this->assertTrue(new SyndPhraseQuery(''), $actual);
	}

	function testPhraseQuery() {
		$this->_index->loadExtension(new SyndPhraseExtension());

		$expected =&
			new SyndBooleanOR(
					new SyndTermQuery('regulation'),
					new SyndPhraseQuery('import export')
					);

		$query = new IndexQuery('regulation || "import export"');
		$actual = $query->createExpression($this->_index);

		$this->assertEquals($expected, $actual);
	}

	function testUnbalancedPhraseQuery() {
		$this->_index->loadExtension(new SyndPhraseExtension());

		$expected =&
			new SyndBooleanOR(
					new SyndBooleanOR(
						new SyndTermQuery('regulation'),
						new SyndTermQuery('import')),
					new SyndTermQuery('export')
					);

		$query = new IndexQuery('regulation || "import export');
		$actual = $query->createExpression($this->_index);

		$this->assertEquals($expected, $actual);
	}
	
	function testNumericQuery() {
		$this->_index->loadExtension($extension = new SyndPhraseExtension());
		$builder = new SqlQueryBuilder($this->_index);
		
		$query = new IndexQuery('1234abc5');
		$expression = $query->createExpression($this->_index);
		$builder->filter($expression->getBalanced());
		
		$hash = $extension->_hash('1234');
		$hash2 = $extension->_hash('abc');
		$context2 = $hash2 | ($hash >> 1);
		
		$expected = "t0.termid = -1679564637 AND t1.termid = 891568578 AND t1.context & $hash = $hash AND t2.termid = -2068763730 AND t2.context & $context2 = $context2";
		$actual = $builder->getWhere();
		
		$this->assertEquals($expected, $actual);
	}

	function testQuotedNumericQuery() {
		$this->_index->loadExtension(new SyndPhraseExtension());
		$query = new IndexQuery('"12345 foo"');
		$expected = new SyndPhraseQuery('1234 5 foo');
		$this->assertEquals($expected, $query->createExpression($this->_index));
	}

	function testExclude() {
		$this->_index->loadExtension(new SyndPhraseExtension());

		$query = new IndexQuery('foo-bar -bar');
		$expected = new SyndBooleanExclude(
			new SyndPhraseQuery('foo bar'),
			new SyndTermQuery('bar'));
		$actual = $query->createExpression($this->_index);
		
		$this->assertEquals($expected, $actual);

		$query = new IndexQuery('-foo-bar');
		$expected = new SyndBooleanExclude(
			null,
			new SyndPhraseQuery('foo bar'));
		$actual = $query->createExpression($this->_index);
		
		$this->assertEquals($expected, $actual);
	}
	
	function testSeparate() {
		$this->_index->loadExtension(new SyndPhraseExtension());

		$query = new IndexQuery('foo123abc && "test321"');
		$expected = new SyndBooleanAND(
			new SyndPhraseQuery('foo 123 abc'),
			new SyndPhraseQuery('test 321'));
		$actual = $query->createExpression($this->_index);
		$this->assertEquals($expected, $actual);

		$query = new IndexQuery('a-bc-123');
		$expected = new SyndPhraseQuery('a bc 123');
		$actual = $query->createExpression($this->_index);
		$this->assertEquals($expected, $actual);

		$query = new IndexQuery('123a4b0');
		$actual = $query->createExpression($this->_index);
		$this->assertEquals('123 a 4 b 0', $actual->toString());
	}
	
	function testSearch() {
		$ext = new SyndPhraseExtension();
		
		$this->_index->loadExtension($ext);
		$this->clearSection($this->_index, '_unit_test');

		$this->_index->addDocument('_unit_test1',
			$this->_index->createComposite(array(
				$this->_index->createFragment('12345 term'),
				$this->_index->createFragment('12345 some terms'))
				),'_unit_test');
		
		$this->_index->addDocument('_unit_test2',
			$this->_index->createFragment('1234'),'_unit_test');
		$this->_index->addDocument('_unit_test3',
			$this->_index->createFragment('foo bar'),'_unit_test');
		$this->_index->addDocument('_unit_test4',
			$this->_index->createFragment('foo no bar'),'_unit_test');
		$this->_index->addDocument('_unit_test5',
			$this->_index->createFragment('some short phrase'),'_unit_test');
		$this->_index->addDocument('_unit_test6',
			$this->_index->createFragment('some short and another short phrase'),'_unit_test');
		$this->_index->addDocument('_unit_test7',
			$this->_index->createFragment('ser1ia456l ABC7654321D'),'_unit_test');
		
		$this->_index->flush();
		
		$mset = $this->_index->getMatchSet(new IndexQuery('12345',array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test1'), $mset);
		
		$mset = $this->_index->getMatchSet(new IndexQuery('1234',array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		sort($mset);
		$this->assertEquals(array('_unit_test1','_unit_test2'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery('foo || bar',array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		sort($mset);
		$this->assertEquals(array('_unit_test3','_unit_test4'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery('"foo bar"',array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test3'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery('"some short"',array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		sort($mset);
		$this->assertEquals(array('_unit_test5','_unit_test6'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery('"some short phrase"',array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test5'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery('ser1ia456l',array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test7'), $mset);

		$query = new IndexQuery('ABC7654321d',array('_unit_test'));
		$expression = $query->createExpression($this->_index);
		$this->assertEquals('abc 7654 321 d', $expression->toString());
		
		$mset = $this->_index->getMatchSet($query, 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test7'), $mset);
	}
}

