<?php
require_once 'unit/index/SyndIndexTestCase.class.inc';
require_once 'core/index/SyndWebIndex.class.inc';
require_once 'core/index/SyndTermIndex.class.inc';
require_once 'core/index/SyndTextFilter.class.inc';
require_once 'core/index/SyndDocumentExtractor.class.inc';
require_once 'core/index/IndexQuery.class.inc';

class _index_WebIndex extends SyndIndexTestCase {
	var $_index = null;
	var $_backend = null;
	
	function setUp() {
		$search = Module::getInstance('search');
		$this->_backend = new SyndTermIndex($search->getIndexDatabase());
		$this->_backend->loadExtension(new SyndTextFilter());
		$this->_backend->loadExtension(new SyndDocumentExtractor());

		$this->_index = new SyndWebIndex($search->getIndexDatabase(), $this->_backend);
	}

	function testSearch() {
		$id = 'NDIwNjdjMzZhYjgMjkuMTkyNzAOTc';
		$uri = "http://www.example.com/$id";
		$uri2 = $uri.'/index2.html';
		
		$title = "Page title";
		$content = "Some text content $id";
		
		$document = $this->_backend->createFragment($title.' '.$id);
		$document2 = $this->_backend->createFragment($title.' '.$content);
		
		$modified = time()-3600*24*5;
		
		$uriParsed = parse_url($uri);
		$uriParsed2 = parse_url($uri2);
		
		$hash = crc32($uriParsed['host'].$uriParsed['path']) ^ crc32($document->toString());
		$hash2 = crc32($uriParsed2['host'].$uriParsed2['path']) ^ crc32($document2->toString());

		$this->_index->addDocument($uri, $title, $document->toString(), 
			$document, strlen($document->toString()), $modified);
		$this->_index->addDocument($uri2, $title, $document2->toString(), 
			$document2, strlen($document2->toString()), $modified);
		
		$expected = array(
			array(
				'DOCID' => (string)crc32($uri),
				'HASH' => (string)$hash,
				'MODIFIED' => (string)$modified,
				'REVISIT' => (string)(time()+3600*24*5),
				'TTL' => (string)(3600*24*5),
				'SIZE' => (string)strlen($document->toString()),
				'FLAGS' => (string)0,
				'URI' => $uri,
				'TITLE' => $title,
				'CONTENT' => $document->toString()),
			array(
				'DOCID' => (string)crc32($uri2),
				'HASH' => (string)$hash2,
				'MODIFIED' => (string)$modified,
				'REVISIT' => (string)(time()+3600*24*5),
				'TTL' => (string)(3600*24*5),
				'SIZE' =>  (string)strlen($document2->toString()),
				'FLAGS' => (string)0,
				'URI' => $uri2,
				'TITLE' => $title,
				'CONTENT' => $document2->toString()),
			);

		$this->_index->flush();

		$actual = $this->_index->getMatchSet(new IndexQuery($id), 0, 10);
		if (1887103736 == $actual[0]['DOCID'])
			$actual = array_reverse($actual);
		$this->assertEquals($expected, $actual);
		
		$this->_diff($expected, $actual);
		
		
		// Test delete document
		$this->_index->delDocument($uri);
		$this->_index->delDocument($uri2);

		$actual = $this->_index->getMatchSet(new IndexQuery($id), 0, 10);
		$this->assertEquals(array(), $actual);
	}
}