<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class SyndIndexTestCase extends PHPUnit2_Framework_TestCase {
	function clearSection($index, $section) {
		assert('!empty($section)');
		if (!empty($section)) {
			$sql = "
				SELECT d.docid FROM synd_search_document d
				WHERE d.section LIKE '$section.%'";
			$index->delDocumentBatch($index->_db->getCol($sql,0));
		}
	}
}
