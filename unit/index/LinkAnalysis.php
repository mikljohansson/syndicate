<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/LinkAnalysisWeightingScheme.class.inc';
require_once 'core/index/SyndNullIndex.class.inc';
require_once 'core/index/SyndDocument.class.inc';
require_once 'core/index/SqlQueryBuilder.class.inc';

class _index_LinkAnalysis extends SyndIndexTestCase {
	function testDocument() {
		$uri = 'http://some.subdomain.example.com/';
		$uri2 = 'http://some.subdomain.example.com/somepage.html';
		$uri3 = 'http://some.subdomain.example.com/someotherpage.html';
		
		$fragment = new SyndDocumentFragment('unit test');
		$document = new SyndDocumentPage(array($fragment));
		$document2 = new SyndDocumentPage(array($fragment), array($uri, $uri3));
		
		$search = Module::getInstance('search');
		$index = new SyndTermIndex($search->getIndexDatabase());
		$index->loadExtension(new LinkAnalysisWeightingScheme());
		$index->addDocument($uri, $document, '_unit_test');
		$index->addDocument($uri2, $document2, '_unit_test');

		$index->flush();
		
		$sql = "
			SELECT d.* FROM synd_search_document d
			WHERE d.docid IN (".crc32($uri).",".crc32($uri2).")";
		$rows = $index->_db->getAll($sql);
		
		$this->assertEquals(2, count($rows));
		
		if ($uri2 == $rows[0]['PAGEID'])
			$rows = array_reverse($rows);
			
		$this->assertEquals(1, $rows[0]['LINKS']);
		$this->assertEquals(2, $rows[1]['LINKS']);
	}
	
	function testQuery() {
		$search = Module::getInstance('search');
		$index = new SyndTermIndex($search->getIndexDatabase());
		$index->loadExtension(new LinkAnalysisWeightingScheme());
		
		$query = new IndexQuery('unittest', array('_unit_test'));
		$expression = $query->createExpression($index);

		$builder = new SqlQueryBuilder($index);
		$index->runHook('query', array($builder, $query, $expression));
		
		$expected = array('WEIGHT' => '1 + d0.rank / 65535');
		$actual = $builder->getColumns();
		$this->assertEquals($expected, $actual);

		$expected = array('WEIGHT' => 'WEIGHT DESC');
		$actual = $builder->getOrderBy();
		$this->assertEquals($expected, $actual);
	}
}

