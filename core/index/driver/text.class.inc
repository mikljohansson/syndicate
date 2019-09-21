<?php
require_once 'core/i18n.inc';
require_once 'core/index/DatabaseIndex.class.inc';
require_once 'core/db/SyndDBLib.class.inc';

/**
 * Fulltext indexing scheme.
 *
 * Implemented using MySQL Fulltext indexes if available or 
 * regular LIKE matching otherwise. MySQL weigh matches according
 * to relevance, other backends has no such features. Use of the 
 * term type indexes is recommended in most cases.
 *
 * @access		public
 * @package		synd.core.index
 */
class synd_index_text extends DatabaseIndex {
	var $_strategy = null;
	var $_dataBuffer = array();

	function __construct($db) {
		parent::__construct($db);
		if (($db instanceof synd_database_mysql))
			$this->_strategy = new SyndMySQLTextIndexStrategy();
		else
			$this->_strategy = new SyndTextIndexStrategy();
	}

	function getMatchSet(IndexQuery $query, $offset, $limit, $order = 'relevance') {
		return $this->_strategy->getMatchSet($this, $query, $offset, $limit);
	}
	
	function getMatchCount(IndexQuery $query) {
		return $this->_strategy->getMatchCount($this, $query);
	}

	function addDocument($pageid, $document, $section = null, $attributes = null) {
		// Add document to buffer
		if (null != ($text = $document->toString())) {
			$this->_dataBuffer[] = array(
				'docid' => $this->documentId($pageId),
				'text' 	=> $text);
		}
		parent::addDocument($pageId, $document, $section, $attributes);
	}

	function delDocument($pageId) {
		$sql = "
			DELETE FROM synd_search_textindex
			WHERE docid = ".$this->documentId($pageId);
		$this->_db->query($sql);
		parent::delDocument($pageId);
	}

	function delDocumentBatch($batch) {
		for ($i=0, $cnt=count($batch); $i<$cnt; $i+=1000) {
			$where = 'docid IN ('.implode(', ', array_slice($batch, $i, 1000)).')';
			$this->_db->deleteQuick('synd_search_textindex', $where);
		}
		parent::delDocumentBatch($batch);
	}

	function clear() {
		$this->_db->truncate('synd_search_textindex');

		unset($this->_dataBuffer);
		$this->_dataBuffer = array();

		parent::clear();
	}

	function clearSection($section) {
		$sql = "
			DELETE FROM synd_search_textindex
			WHERE docid IN (
				SELECT d.docid FROM synd_search_document d
				WHERE 
					d.section = '$section' OR
					d.section LIKE '$section.%')";
		$this->_db->query($sql);
		parent::clearSection($section);
	}
	
	function _flush() {
		// Flush parent
		if (SyndLib::isError($result = parent::_flush()))
			return $result;

		// Flush page data
		if (SyndLib::isError($result = $this->_db->load('synd_search_textindex', $this->_dataBuffer)))
			return $result;

		unset($this->_dataBuffer);
		$this->_dataBuffer = array();
	}
	
	function analyze() {
		parent::analyze();
		$this->_db->analyze('synd_search_textindex');
	}

	function optimize() {
		parent::optimize();
		$this->_db->optimize('synd_search_textindex');
	}
}

/**
 * Textindex search strategy interface
 * @abstract
 * @access		public
 * @package		synd.core.index
 */
class ISyndTextIndexStrategy {
	function getMatchSet(synd_index_text $index, IndexQuery $query, $offset, $limit) {}
	function getMatchCount(synd_index_text $index, IndexQuery $query) {}
}

/**
 * MySQL FullText index.
 *
 * Uses the MySQL FullText index feature to perform searches. This
 * index type can weigh mathes according to their relevance and
 * supports boolean queries.
 *
 * @access		protected
 */
class SyndMySQLTextIndexStrategy extends ISyndTextIndexStrategy {
	function _getMatch(synd_index_text $index, $text) {
//		global $SYND_WORDCHARS;
//		if (preg_match("/[^{$SYND_WORDCHARS}][+\-><()~*:\"&|][{$SYND_WORDCHARS}]/", $text))
//			return "MATCH(ti.text) AGAINST (".$index->_db->quote($text)." IN BOOLEAN MODE)";
		return "MATCH(ti.text) AGAINST (".$index->_db->quote($text).")";
	}

	function getMatchSet(synd_index_text $index, IndexQuery $query, $offset, $limit) {
		$match = $this->_getMatch($index, $query->getText($index));
		$sql = "
			SELECT d.DOCID, d.PAGEID, d.SECTION, $match WEIGHT
			FROM synd_search_document d, synd_search_textindex ti
			WHERE ".$index->getSectionFilter($query->getSections())."
				d.docid = ti.docid AND $match
			ORDER BY weight DESC
			LIMIT $offset, $limit";
			
		if (SyndLib::isError($rows = $index->_db->getAll($sql)))
			return trigger_error("DB Error: $sql", E_USER_WARNING);
		return $rows;
	}

	function getMatchCount(synd_index_text $index, IndexQuery $query) {
		$match = $this->_getMatch($index, $query->getText($index));
		$sql = "
			SELECT COUNT(d.docid)
			FROM synd_search_document d, synd_search_textindex ti
			WHERE ".$index->getSectionFilter($query->getSections())."
				d.docid = ti.docid AND $match";
		
		if (SyndLib::isError($count = $index->_db->getOne($sql)))
			return trigger_error("DB Error: $sql", E_USER_WARNING);
		return $count;
	}
}

/**
 * Regular 'LIKE' text searching
 * @access		protected
 */
class SyndTextIndexStrategy extends ISyndTextIndexStrategy {
	function getMatchSet(synd_index_text $index, IndexQuery $query, $offset, $limit) {
		$filter = SyndDBLib::sqlLikeExpr($query->getText($index), 
			array('ti.text'), 'AND', 2, false);
		if (null == $filter)
			return array();

		$sql = "
			SELECT d.DOCID, d.PAGEID, d.SECTION, 1 WEIGHT
			FROM synd_search_document d, synd_search_textindex ti
			WHERE ".$index->getSectionFilter($query->getSections())."
				d.docid = ti.docid AND $filter";

		if (SyndLib::isError($rows = $index->_db->getAll($sql, $offset, $limit)))
			return trigger_error("DB Error: $sql", E_USER_WARNING);
		return $rows;
	}
	
	function getMatchCount(synd_index_text $index, IndexQuery $query) {
		$filter = SyndDBLib::sqlLikeExpr($query->getText($index), 
			array('ti.text'), 'AND', 2, false);
		if (null == $filter)
			return array();

		$sql = "
			SELECT COUNT(d.docid)
			FROM synd_search_document d, synd_search_textindex ti
			WHERE ".$index->getSectionFilter($query->getSections())."
				d.docid = ti.docid AND $filter";
		
		if (SyndLib::isError($count = $index->_db->getOne($sql)))
			return trigger_error("DB Error: $sql", E_USER_WARNING);
		return $count;
	}
}

