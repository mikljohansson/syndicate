<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/IndexQuery.class.inc';
require_once 'core/index/SyndTermIndex.class.inc';
require_once 'core/index/SyndNullIndex.class.inc';
require_once 'core/index/SyndTextFilter.class.inc';
require_once 'core/index/SyndPhraseExtension.class.inc';
require_once 'core/index/BM25WeightingScheme.class.inc';
require_once 'core/index/SyndRangeExtension.class.inc';

class _index_RangeExtension extends SyndIndexTestCase {
	function setUp() {
		$search = Module::getInstance('search');
		$this->_index = new SyndTermIndex($search->getIndexDatabase());
		$this->_index->loadExtension(new SyndTextFilter());
		$this->_index->loadExtension(new SyndPhraseExtension());
		$this->_index->loadExtension(new BM25WeightingScheme());
	}

	function testLexerExpression() {
		$this->_index->loadExtension(new SyndRangeExtension());
		$this->assertTrue(in_array('\.{2,3}',$this->_index->lexerExpression()));
	}

	function testExpression() {
		$actual = $this->_index->createExpression('..');
		$this->assertFalse(($actual instanceof SyndRangeQuery));

		$this->_index->loadExtension(new SyndRangeExtension());

		$actual = $this->_index->createExpression('..');
		$this->assertEquals(new SyndRangeQuery(null,null), $actual);
	}

	function testParser() {
		$this->_index->loadExtension(new SyndRangeExtension());

		$expected =&
			new SyndRangeQuery(
					new SyndTermQuery('12345'),
					new SyndTermQuery('67890')
					);

		$query = new IndexQuery('12345..67890');
		$actual = $query->createExpression($this->_index);

		$this->assertEquals($expected, $actual);
	}

	function testInvalidParser() {
		$this->_index->loadExtension(new SyndRangeExtension());

		$expected =&
			new SyndBooleanAND(
					new SyndTermQuery('abc'),
					new SyndTermQuery('456')
					);

		$query = new IndexQuery('abc..456');
		$actual = $query->createExpression($this->_index);

		$this->assertEquals($expected, $actual);
	}

	function testBounded() {
		$expression = new SyndRangeQuery(new SyndTermQuery('12345'), new SyndTermQuery('67890'));
		$this->assertTrue($expression->isBounded());

		$expression = new SyndRangeQuery(new SyndTermQuery('12345'), null);
		$this->assertFalse($expression->isBounded());

		$expression = new SyndRangeQuery(null, new SyndTermQuery('12345'));
		$this->assertFalse($expression->isBounded());

		$expression = new SyndRangeQuery(null, null);
		$this->assertFalse($expression->isBounded());
	}

	function testDateFormats() {
		$extension = new SyndRangeExtension();
		$this->_index->loadExtension($extension);

		$this->_format($extension, '2005-01-31', '20050131');
		$this->_format($extension, '2005:01:31', '20050131');
		$this->_format($extension, '2005-01-31T08:00:00', '20050131');
		$this->_format($extension, '2005-01-31T08:00:00Z', '20050131');
		$this->_format($extension, '05-01-31', '20050131');
		$this->_format($extension, '01/31/05', '20050131');
	}
	
	function _format($extension, $text, $expected, $line) {
		$this->assertEquals($expected, $extension->_parse($text), $line);

		$extension->_numerics = array();
		$extension->_index->addDocument(md5(uniqid('')), 
			new SyndDocumentFragment($text), '_unit_test');
		
		$parsed = array_shift($extension->_numerics);
		$this->assertEquals($expected, $parsed['term'], $line);
		
		$numerics = SyndLib::array_collect($extension->_numerics, 'term');
		sort($numerics);
		preg_match_all('/\d+/', $text, $matches);
		$expected = array_unique($matches[0]);
		sort($expected);

		$this->_diff($expected, $numerics);
		$this->assertEquals($expected, $numerics, $line);
	}
	
	function testRangeQuery() {
		$extension = new SyndRangeExtension();
		$this->_index->loadExtension($extension);
		
		$query = new IndexQuery('123..456');
		$builder = new SqlQueryBuilder($this->_index);
		$expression = $query->createExpression($this->_index);
		
		$expected = "(tn0.term BETWEEN 123 AND 456)";
		$actual = $builder->parseExpression($expression->getBalanced());
		
		$this->assertEquals($expected, $actual);
	}
	
	function testDateRangeQuery() {
		$extension = new SyndRangeExtension();
		$this->_index->loadExtension($extension);
		
		$query = new IndexQuery('2003-04-05..2004-06-07');
		$builder = new SqlQueryBuilder($this->_index);
		$expression = $query->createExpression($this->_index);
		
		$expected = "(tn0.term BETWEEN 20030405 AND 20040607)";
		$actual = $builder->parseExpression($expression->getBalanced());
		
		$this->assertEquals($expected, $actual);
	}

	function testSearch() {
		$extension = new SyndRangeExtension();
		$this->_index->loadExtension($extension);
		$this->clearSection($this->_index, '_unit_test');

		$id = preg_replace('/[^a-z]/i','', base64_encode(uniqid('',true)));
		
		$this->_index->addDocument('_unit_test1',
			$this->_index->createFragment('23456'),'_unit_test');
		$this->_index->addDocument('_unit_test2',
			$this->_index->createFragment("120 $id"),'_unit_test');
		$this->_index->addDocument('_unit_test3',
			$this->_index->createFragment("20 $id"),'_unit_test');
		$this->_index->addDocument('_unit_test4',
			$this->_index->createFragment('2003-05-08'),'_unit_test');
		
		$this->_index->flush();
		
		$mset = $this->_index->getMatchSet(new IndexQuery('12345..45678',array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test1'), $mset);
		
		$mset = $this->_index->getMatchSet(new IndexQuery('100..150',array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test2'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery('1..40000',array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		sort($mset);
		$this->assertEquals(array('_unit_test1','_unit_test2','_unit_test3','_unit_test4'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery("$id && 1..150",array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		sort($mset);
		$this->assertEquals(array('_unit_test2', '_unit_test3'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery("$id && 1..50",array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test3'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery('1..4',array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array(), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery('2003-04-05..2004-06-07',array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test4'), $mset);
		
		// Check that the 'index_clear_section' hook works
		$sql = "
			SELECT d.docid FROM synd_search_document d
			WHERE d.section = '_unit_test.'";
		$docId = $this->_index->_db->getOne($sql);
		$this->assertNotNull($docId);
		
		$sql2 = "
			SELECT 1 FROM synd_search_numeric n
			WHERE n.docid = $docId";
		$this->assertNotNull($this->_index->_db->getOne($sql2));

		$this->clearSection($this->_index, '_unit_test');

		$this->assertNull($this->_index->_db->getOne($sql));
		$this->assertNull($this->_index->_db->getOne($sql2));
	}
}
