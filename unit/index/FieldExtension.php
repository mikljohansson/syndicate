<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/IndexQuery.class.inc';
require_once 'core/index/SyndTermIndex.class.inc';
require_once 'core/index/SyndNullIndex.class.inc';
require_once 'core/index/SyndTextFilter.class.inc';
require_once 'core/index/SyndPhraseExtension.class.inc';
require_once 'core/index/SyndFieldExtension.class.inc';
require_once 'core/index/SyndRangeExtension.class.inc';

class _index_FieldExtension extends SyndIndexTestCase {
	var $_index = null;

	function setUp() {
		$search = Module::getInstance('search');
		$this->_index = new SyndTermIndex($search->getIndexDatabase());
		$this->_index->loadExtension(new SyndTextFilter());
		$this->_index->loadExtension(new SyndPhraseExtension());
	}

	function testLexerExpression() {
		$this->_index->loadExtension(new SyndFieldExtension());
		$this->assertTrue(in_array(':',$this->_index->lexerExpression()));
	}

	function testExpression() {
		$actual = $this->_index->createExpression(':');
		$this->assertFalse(($actual instanceof SyndFieldQuery));

		$this->_index->loadExtension(new SyndFieldExtension());

		$actual = $this->_index->createExpression(':');
		$this->assertEquals(new SyndFieldQuery(), $actual);
	}

	function testParser() {
		$this->_index->loadExtension(new SyndFieldExtension());

		$expected =&
			new SyndFieldQuery(
					new SyndTermQuery('title'),
					new SyndTermQuery('import')
					);

		$query = new IndexQuery('title:import');
		$actual = $query->createExpression($this->_index);
		$this->assertEquals($expected, $actual);

		$expected = new SyndBooleanAND(
			new SyndTermQuery('foo'),
			new SyndFieldQuery(
					new SyndTermQuery('title'),
					new SyndTermQuery('import')));

		$query = new IndexQuery('foo && title:import');
		$actual = $query->createExpression($this->_index);
		$this->assertEquals($expected, $actual);

		$expected = new SyndBooleanExclude(
			new SyndTermQuery('foo'),
			new SyndFieldQuery(
					new SyndTermQuery('title'),
					new SyndTermQuery('import')));

		$query = new IndexQuery('foo && -title:import');
		$actual = $query->createExpression($this->_index);
		$this->assertEquals($expected, $actual);
	}
	
	function testFieldQuery() {
		$extension = new SyndFieldExtension();
		$this->_index->loadExtension($extension);
		
		$query = new IndexQuery('title:import');
		$builder = new SqlQueryBuilder($this->_index);
		$expression = $query->createExpression($this->_index);
		
		$hash = $extension->_hash('title');
		$expected = "(".$this->_index->_db->bitand('t0.field', $hash)." = $hash AND t0.termid = -1655779811)";
		$actual = $builder->parseExpression($expression->getBalanced());
		
		$this->assertEquals($expected, $actual);
	}
	
	function testBounded() {
		$expression = new SyndFieldQuery(
			new SyndTermQuery('title'),
			new SyndTermQuery('import'));
		$this->assertTrue($expression->isBounded());

		$expression = new SyndFieldQuery(
			new SyndTermQuery('filetype'),
			new SyndTermQuery('pdf'));
		$this->assertFalse($expression->isBounded());

		$expression = new SyndFieldQuery(
			new SyndTermQuery('title'),
			new SyndRangeQuery(
				new SyndTermQuery('12345'),
				null));
		$this->assertFalse($expression->isBounded());
	}
	
	function testDocument() {
		$extension = new SyndFieldExtension();
		$this->_index->loadExtension($extension);

		$document = $this->_index->createComposite(array(
			$this->_index->createFragment('import'),
			$this->_index->createFragment('export','body')),
			'title');
	
		$this->_index->addDocument('_unit_test', $document);
		
		$exportPost = array_pop($this->_index->_postings);
		$importPost = array_pop($this->_index->_postings);
		
		$this->assertEquals($extension->_hash('title'), $importPost['field']);
		$this->assertEquals($extension->_hash('title'), $exportPost['field']);
	}
	
	function testAliases() {
		$extension = new SyndFieldExtension();
		$extension->addFieldAlias('title', 'titel');
		
		$hash = $extension->_hash('title');
		$this->assertEquals($hash, $extension->_hash('titel'));
		
		$this->assertTrue(null != $extension->_hash('INFO_HEAD'));
	}
	
	function testHeight() {
		$extension = new SyndFieldExtension();
		$this->_index->loadExtension($extension);

		$query = new IndexQuery('title:foo');
		$expression = $query->createExpression($this->_index);
		$this->assertTrue(($expression instanceof SyndFieldQuery));
		$this->assertEquals(1, $expression->getHeight());

		$query = new IndexQuery('title:"foo bar"');
		$expression = $query->createExpression($this->_index);
		$this->assertEquals(2, $expression->getHeight());
	}
	
	function testSearch() {
		$extension = new SyndFieldExtension();
		$this->_index->loadExtension($extension);
		$this->clearSection($this->_index, '_unit_test');
		
		$id = preg_replace('/[^a-z]/i','', base64_encode(uniqid('',true)));
		$id2 = preg_replace('/[^a-z]/i','', base64_encode(uniqid('',true)));
		$id3 = preg_replace('/[^a-z]/i','', base64_encode(uniqid('',true)));

		$this->_index->addDocument('_unit_test1',
			$this->_index->createComposite(array(
				$this->_index->createFragment($id, 'title'),
				$this->_index->createFragment('foo'),
			)),
			'_unit_test');

		$this->_index->addDocument('_unit_test2',
			$this->_index->createComposite(array(
				$this->_index->createFragment($id, 'body'),
				$this->_index->createFragment('foo'),
				)),
			'_unit_test');

		$this->_index->addDocument('_unit_test3',
			$this->_index->createComposite(array(
				$this->_index->createFragment($id, 'title'),
				$this->_index->createFragment($id, 'body'),
				)), 
			'_unit_test');

		$this->_index->addDocument('_unit_test4',
			$this->_index->createComposite(array(
				$this->_index->createFragment($id2, 'title'),
				$this->_index->createFragment($id3),
				)), 
			'_unit_test');

		$this->_index->addDocument('_unit_test5',
			$this->_index->createComposite(array(
				$this->_index->createFragment($id2.' '.$id3, 'title'),
				)), 
			'_unit_test');

		$this->_index->flush();
		
		$mset = $this->_index->getMatchSet(new IndexQuery($id), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		sort($mset);
		$this->assertEquals(array('_unit_test1','_unit_test2','_unit_test3'), $mset);
		
		$mset = $this->_index->getMatchSet(new IndexQuery("title:$id", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		sort($mset);
		$this->assertEquals(array('_unit_test1','_unit_test3'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery("body:$id", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		sort($mset);
		$this->assertEquals(array('_unit_test2','_unit_test3'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery("foo && title:$id", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test1'), $mset);

		$mset = $this->_index->getMatchSet(new IndexQuery("foo && -title:$id", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test2'), $mset);
		
		$mset = $this->_index->getMatchSet(new IndexQuery("title:($id2 && $id3)", array('_unit_test')), 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test5'), $mset);
	}
	
	function testSort() {
		$extension = new SyndFieldExtension(true);
		$this->_index->loadExtension($extension);
		$this->clearSection($this->_index, '_unit_test');

		$id = preg_replace('/[^a-z]/i','', base64_encode(uniqid('',true)));

		$this->_index->addDocument('_unit_test1',
			$this->_index->createComposite(array(
				$this->_index->createFragment($id),
				$this->_index->createFragment('1','title'),
				$this->_index->createFragment('5','description')
			)), '_unit_test');

		$this->_index->addDocument('_unit_test2',
			$this->_index->createComposite(array(
				$this->_index->createFragment($id),
				$this->_index->createFragment('2','title'),
				$this->_index->createFragment('4','description')
			)), '_unit_test');

		$this->_index->addDocument('_unit_test3',
			$this->_index->createComposite(array(
				$this->_index->createFragment($id),
				$this->_index->createFragment('2','title'),
				$this->_index->createFragment('6','description')
			)), '_unit_test');
			
		$this->_index->flush();
	
		$query = new IndexQuery($id);
		$query->order('title');
		$query->order('description');
		
		$mset = $this->_index->getMatchSet($query, 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test1','_unit_test2','_unit_test3'), $mset);


		$query = new IndexQuery($id);
		$query->order('title');
		$query->order('description',false);
		
		$mset = $this->_index->getMatchSet($query, 0, 10);
		$mset = SyndLib::array_collect($mset, 'ID');
		$this->assertEquals(array('_unit_test1','_unit_test3','_unit_test2'), $mset);



		// Check that the 'index_clear_section' hook works
		$sql = "
			SELECT d.docid FROM synd_search_document d
			WHERE d.section = '_unit_test.'";
		$docId = $this->_index->_db->getOne($sql);
		$this->assertNotNull($docId);
		
		$sql2 = "
			SELECT 1 FROM synd_search_field n
			WHERE n.docid = $docId";
		$this->assertNotNull($this->_index->_db->getOne($sql2));

		$this->clearSection($this->_index, '_unit_test');

		$this->assertNull($this->_index->_db->getOne($sql));
		$this->assertNull($this->_index->_db->getOne($sql2));
	}
}

