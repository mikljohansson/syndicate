<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/SyndTermIndex.class.inc';
require_once 'core/index/SyndTextFilter.class.inc';
require_once 'core/index/SyndStemFilter.class.inc';
require_once 'core/index/SyndPhraseExtension.class.inc';
require_once 'core/index/SyndRangeExtension.class.inc';
require_once 'core/index/SyndPartialExtension.class.inc';
require_once 'core/index/BM25WeightingScheme.class.inc';
require_once 'core/index/IndexQuery.class.inc';

class _index_TermIndex extends SyndIndexTestCase {
	var $_index = null;

	function setUp() {
		$search = Module::getInstance('search');
		$this->_index = new SyndTermIndex($search->getIndexDatabase());
		
		$this->_index->loadExtension(new SyndTextFilter());
		$this->_index->loadExtension(new SyndStemFilter('en'));
		$this->_index->loadExtension(new SyndPhraseExtension());
		$this->_index->loadExtension(new SyndRangeExtension());
//		$this->_index->loadExtension(new SyndPartialExtension());
		$this->_index->loadExtension(new BM25WeightingScheme());
	}
	
	function testMatchSet() {
		$query = new IndexQuery('"barnen kunde" följa');
		
		$mset = $this->_index->getMatchSet($query, 0, 100);
		$this->assertFalse(SyndLib::isError($mset));
		$mset = $this->_index->getMatchSet($query,0,10);
		$this->assertFalse(SyndLib::isError($mset));
		$mset = $this->_index->getMatchSet($query,5,10);
		$this->assertFalse(SyndLib::isError($mset));
	}

	function testMatchCount() {
		$query = new IndexQuery('"barnen kunde" följa');

		$mset = $this->_index->getMatchSet($query, 0, 100);
		$count = $this->_index->getMatchCount($query);

		$this->assertFalse(SyndLib::isError($count));
		$this->assertEquals(count($mset), $count);
	}

	function testExpandSet() {
		$query = new IndexQuery('"barnen kunde" följa');

		$mset = $this->_index->getMatchSet($query, 0, 100);
		$rset = array_slice(SyndLib::array_collect($mset,'ID'),0,2);
		
		$eset = $this->_index->getExpandSet(new IndexQuery(null,null,$rset));
		$this->assertFalse(SyndLib::isError($eset));
	}
	
	function testFragment() {
		$str = 'fragmented fragment 99AB1C23.';
		$document = $this->_index->createFragment($str);
		$this->_index->addDocument('unit_test.1', $document, '_unit_test');
		
		$expected = array(
			crc32('23') => array(
				'TERMID' => crc32('23'),
				'TERM' => "'23'",
				'ORIGINAL' => "'23'",
				'FUZZY' => 'NULL',
				'N' => 1),
			crc32('1') => array(
				'TERMID' => crc32('1'),
				'TERM' => "'1'",
				'ORIGINAL' => "'1'",
				'FUZZY' => 'NULL',
				'N' => 1),
			crc32('c') => array(
				'TERMID' => crc32('c'),
				'TERM' => "'c'",
				'ORIGINAL' => "'c'",
				'FUZZY' => 'NULL',
				'N' => 1),
			crc32('fragment') => array(
				'TERMID' => crc32('fragment'),
				'TERM' => "'fragment'",
				'ORIGINAL' => "'fragmented'",
				'FUZZY' => 'NULL',
				'N' => 1),
			crc32('99') => array(
				'TERMID' => crc32('99'),
				'TERM' => "'99'",
				'ORIGINAL' => "'99'",
				'FUZZY' => 'NULL',
				'N' => 1),
			crc32('ab') => array(
				'TERMID' => crc32('ab'),
				'TERM' => "'ab'",
				'ORIGINAL' => "'ab'",
				'FUZZY' => 'NULL',
				'N' => 1),
			);

		$terms = $this->_index->_terms;
		foreach (array_keys($terms) as $key)
			unset($terms[$key]['n']);

		$this->assertEquals($expected, $terms);
		
		$expected = array(
			crc32('unit_test.1').'.fragment' => array(
				'docid' => crc32('unit_test.1'),
				'termid' => crc32('fragment'),
				'field' => 0,
				'context' => 537,
				'wdf' => 2,
				'wdw' => (float)255,
				),
			crc32('unit_test.1').'.99' => array(
				'docid' => crc32('unit_test.1'),
				'termid' => crc32('99'),
				'field' => 0,
				'context' => 797,
				'wdf' => 1,
				'wdw' => (float)255,
				),
			crc32('unit_test.1').'.ab' => array(
				'docid' => crc32('unit_test.1'),
				'termid' => crc32('ab'),
				'field' => 0,
				'context' => 25868,
				'wdf' => 1,
				'wdw' => (float)255,
				),
			crc32('unit_test.1').'.1' => array(
				'docid' => crc32('unit_test.1'),
				'termid' => crc32('1'),
				'field' => 0,
				'context' => 29441,
				'wdf' => 1,
				'wdw' => (float)255,
				),
			crc32('unit_test.1').'.c' => array(
				'docid' => crc32('unit_test.1'),
				'termid' => crc32('c'),
				'field' => 0,
				'context' => 13464,
				'wdf' => 1,
				'wdw' => (float)255,
				),
			crc32('unit_test.1').'.23' => array(
				'docid' => crc32('unit_test.1'),
				'termid' => crc32('23'),
				'field' => 0,
				'context' => 19020,
				'wdf' => 1,
				'wdw' => (float)255,
				),
			0 => array(
				'docid' => crc32('unit_test.1'),
				'termid' => 0,
				'field' => 0,
				'context' => 0,
				'wdf' => 0,
				'wdw' => 0,
				),
			);

		$this->assertEquals($expected, $this->_index->_postings);
		//$this->_diff($expected, $this->_index->_postings);
		
		$this->_index->flush();
	}
	
	function testExcludeParse() {
		$query = new IndexQuery("foo -bar");

		$expression = new SyndBooleanExclude(
			new SyndTermQuery('foo'), new SyndTermQuery('bar'));
		$actual = $query->createExpression($this->_index);
		$this->assertEquals($expression, $actual);

		$builder = new SqlQueryBuilder($this->_index);
		$expected = "t0.termid = -1938594527";
		$actual = $builder->parseExpression($expression->getBalanced());
		$this->assertEquals($expected, $actual);

		$expected = "synd_search_termindex t0, synd_search_document d0
			LEFT JOIN (
				SELECT sd0.docid FROM synd_search_document sd0, synd_search_termindex st0
				WHERE st0.termid = 1996459178 AND sd0.docid = st0.docid 
LIMIT 0, 25000) tsd0 
			ON (tsd0.docid = d0.docid)";
		
		$this->assertEquals($expected, $builder->getFrom());
	}

	function testSearch() {
		$this->clearSection($this->_index, '_unit_test');
		$id = '43cff030cecc6afedklakjkghcb587ac';

		$this->_index->addDocument('_unit_test1',
			$this->_index->createFragment("$id foo"),
			'_unit_test');
		$this->_index->addDocument('_unit_test2',
			$this->_index->createFragment("$id bar"),
			'_unit_test');
		$this->_index->addDocument('_unit_test3',
			$this->_index->createFragment("123a4b5"),
			'_unit_test');
		$this->_index->addDocument('_unit_test4',
			$this->_index->createFragment("123"),
			'_unit_test');
		$this->_index->addDocument('_unit_test5',
			$this->_index->createFragment("a2abc"),
			'_unit_test');
			
		$this->_index->flush();
		
		// Test term frequency 
		$sql = "SELECT td.n FROM synd_search_term td WHERE td.termid = ".$this->_index->termId("afedklakjkghcb");
		$freq = $this->_index->_db->getOne($sql);
		$this->assertEquals(2, $freq);
		
		$mset = $this->_index->getMatchSet(new IndexQuery("$id", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		sort($mset);
		$this->assertEquals(array('_unit_test1','_unit_test2'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery("+$id -bar", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test1'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery("123", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		sort($mset);
		$this->assertEquals(array('_unit_test3','_unit_test4'), $mset);
		
		$mset = $this->_index->getMatchSet(new IndexQuery("123a4b5", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test3'), $mset);
		
		$mset = $this->_index->getMatchSet(new IndexQuery("a2abc", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test5'), $mset);

		// Test term frequency on delete
		$this->_index->delDocument('_unit_test1');
		$this->_index->delDocument('_unit_test2');
		
		$this->_index->addDocument('_unit_test1',
			$this->_index->createFragment("$id foo"),
			'_unit_test');
		$this->_index->addDocument('_unit_test2',
			$this->_index->createFragment("$id bar"),
			'_unit_test');

		$this->_index->flush();
		
		$freq = $this->_index->_db->getOne($sql);
		$this->assertEquals(2, $freq);

		
		// Test the delDocumentBatch() function here to save time
		$batch = $this->_index->documentId(array('_unit_test1', '_unit_test2'));
		$this->_index->delDocumentBatch($batch);
		
		$mset = $this->_index->getMatchSet(new IndexQuery("$id", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array(), $mset);

		// Test term deletion when N reaches 0
		$this->_index->flush();
		$freq = $this->_index->_db->getOne($sql);
		$this->assertEquals(null, $freq);
	}
}

