<?php
require_once 'core/index/AbstractIndex.class.inc';
require_once 'core/db/Database.class.inc';

/**
 * Database stored document index.
 * @package		synd.core.index
 */
abstract class DatabaseIndex extends AbstractIndex {
	/**
	 * Database handle
	 * @var		object	Database
	 * @access	protected
	 */
	var $_db = null;

	/**
	 * Current document context.
	 *
	 * Used to communicate the context when indexing a document
	 * throught addDocument(), visitDocumentFragment() calls.
	 *
	 * @var		array
	 * @access	protected
	 */
	var $_context = array();

	/**
	 * Document buffer
	 * @access	private
	 */
	var $_documents = array();
	
	/**
	 * Document attributes buffer
	 * @access	private
	 */
	var $_attributes = array();

	/**
	 * Minimum NDL value.
	 *
	 * Specifies a cutoff on the minimum value that can be used for a 
	 * normalised document length - smaller values will be forced up to 
	 * this cutoff. This prevents very small documents getting a huge 
	 * bonus weight. 
	 *
	 * @var		float
	 * @access	protected
	 */
	var $_ndlMinimum = 0.25;
	
	/**
	 * @access	private
	 */
	var $_documentCount = null;

	/**
	 * Constructor
	 * @param	mixed	'object Database' or 'string DSN' of database to use
	 */
	function __construct($db) {
		parent::__construct();
		if (is_object($db)) 
			$this->_db = $db;
		else if (SyndLib::isError($this->_db = DatabaseManager::getConnection($db))) {
			trigger_error($this->_db->getMessage(), E_USER_WARNING);
			$this->_db = DatabaseManager::getConnection('null://null');
		}
	}
	
	function sectionFilter($sections, $d) {
		if (empty($sections))
			return null;

		$sql = null;
		foreach ($sections as $section) {
			if (null != $sql)
				$sql .= ' OR ';
			$sql .= "$d.section LIKE '$section.%'";
		}
		
		return "($sql)";
	}

	function attributeFilter($attributes, $a) {
		if (empty($attributes))
			return null;

		$sql = null;
		foreach ($attributes as $attribute) {
			if (null != $sql)
				$sql .= ' OR ';
			$sql .= "$a.attribute LIKE '$attribute.%'";
		}
		
		return "($sql)";
	}

	function getDocumentCount($section = null) {
		if (null == $section && null !== $this->_documentCount)
			return $this->_documentCount;
		
		$sql = "SELECT COUNT(*) FROM synd_search_document d";
		if (null != $section && null != ($filter = $this->sectionFilter(array($section), 'd')))
			$sql .= " WHERE $filter";
		if (SyndLib::isError($documents = $this->_db->getOne($sql)))
			return trigger_error($sql, E_USER_WARNING);

		if (null == $section)
			$this->_documentCount = $documents;
		return $documents;
	}

	function documentId($pageid) {
		if (!is_array($pageid))
			return crc32($pageid);
		foreach ($pageid as $key => $value)
			$pageid[$key] = $this->documentId($value);
		return $pageid;
	}
	
	function addDocument($pageid, $document, $section = null, $attributes = null) {
		assert('null != $pageid && (is_scalar($section) || null === $section)');
		profiler_enter(__METHOD__);
		
		$docid = crc32($pageid);
		$this->_context['docid'] = $docid;
		$this->_context['pageid'] = $pageid;
		$this->_context['section'] = substr($section, 0, 128);
		
		// Add document to buffer
		$this->_documents[$docid] = array(
			'docid'   => $docid,
			'pageid'  => $this->_db->quote($pageid),
			'section' => $this->_db->quote($this->_context['section'].'.'),
			'modified'=> $document->getModified() ? $document->getModified() : time(),
			'length'  => 0,
			'rank'    => 1,
			'links'   => 1);

		// Add document attributes to buffer
		if (!empty($attributes)) {
			foreach ($attributes as $attribute) {
				$this->_attributes[$docid.'.'.$attribute] = array(
					'docid'		=> $docid,
					'attribute'	=> $this->_db->quote($attribute.'.'));
			}
		}

		parent::addDocument($pageid, $document, $section, $attributes);
		if (count($this->_documents) >= 750 && SyndLib::isError($result = $this->flush()))
			trigger_error($result->getMessage(), E_USER_ERROR);
		
		profiler_leave(__METHOD__);
	}

	function getCurrentSection() {
		return $this->_context['section'];
	}
	
	function getCurrentDocumentId() {
		return $this->_context['docid'];
	}

	function delDocument($pageid) {
		parent::delDocument($pageid);
		$sql = "
			DELETE FROM synd_search_document
			WHERE docid = ".$this->documentId($pageid);
		$this->_db->query($sql);
	}
	
	function delDocumentBatch($batch) {
		parent::delDocumentBatch($batch);
		for ($i=0, $cnt=count($batch); $i<$cnt; $i+=1000) {
			$where = 'docid IN ('.implode(',', $this->_db->quote(array_slice($batch, $i, 1000))).')';
			$this->_db->deleteQuick('synd_search_document', $where);
		}
	}

	function clear() {
		parent::clear();
		$this->_db->truncate('synd_search_document');
		unset($this->_documents);
		$this->_documents = array();
	}

	function clearSection($section) {
		parent::clearSection($section);
		$this->_db->deleteQuick('synd_search_document', "section LIKE '$section.%'");
	}
	
	/**
	 * Update Normalized Document Length (ndl)
	 */
	function commit() {
		parent::commit();

		$sql = "SELECT AVG(d.length) FROM synd_search_document d";
		$avgLength = $this->_db->getOne($sql);

		$sql = "
			UPDATE synd_search_document
			SET ndl = ".$this->_db->ternary("length/$avgLength < ".$this->_ndlMinimum, 
				$this->_ndlMinimum, "length/$avgLength")."
			WHERE docid != 0";
		if (SyndLib::isError($result = $this->_db->query($sql)))
			return $result;
	}
	
	/**
	 * Flushes the document buffers to database
	 */
	function _flush() {
		// If index is online, adjust normalized document length (ndl)
		if (!$this->inTransaction() && !empty($this->_documents)) {
			$avgLength = array_sum(SyndLib::array_collect($this->_documents, 'length')) / count($this->_documents);

			$sql = "SELECT AVG(d.length) FROM synd_search_document d";
			if (null != ($length = $this->_db->getOne($sql)))
				$avgLength = ($avgLength + $length) / 2;
			
			foreach (array_keys($this->_documents) as $docid) {
				$this->_documents[$docid]['ndl'] = 
					max($this->_ndlMinimum, $this->_documents[$docid]['length'] / $avgLength);
			}
		}
		
		// Clear out old documents
		for ($i=0, $cnt=count($this->_documents); $i<$cnt; $i+=1000) {
			$where = 'docid IN ('.implode(',', array_slice(array_keys($this->_documents), $i, 1000)).')';
			$this->_db->deleteQuick('synd_search_document', $where);
		}
		
		// Load documents into table
		if (SyndLib::isError($result = $this->_db->load('synd_search_document', $this->_documents)))
			return $result;

		// Load document attributes into table
		if (SyndLib::isError($result = $this->_db->load('synd_search_attribute', $this->_attributes)))
			return $result;

		// Flush parent
		parent::_flush();
		
		// Deallocate memory properly
		unset($this->_documents, $this->_attributes);
		$this->_documents = array();
		$this->_attributes = array();
	}

	/**	
	 * Analyze the key distribution on the indexes
	 * @see		DatabaseConnection::analyze()
	 */
	function analyze() {
		parent::analyze();
		$this->_db->analyze('synd_search_document');
	}
	
	/**
	 * Rebuild the indexes to reclaim wasted space
	 * @see		DatabaseConnection::optimize()
	 */
	function optimize() {
		parent::optimize();
		$this->_db->optimize('synd_search_document');
	}
}
