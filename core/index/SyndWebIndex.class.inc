<?php
require_once 'core/index/SyndSpider.class.inc';
require_once 'core/index/SyndExpression.class.inc';
require_once 'core/index/SyndIndexExtension.class.inc';
require_once 'core/db/Database.class.inc';

define('SYND_ENCODING_GZIP', 1);

/**
 * Webpage index
 *
 * @access		public
 * @package		synd.core.index
 */
class SyndWebIndex {
	/**
	 * Database backend
	 * @var		object	Database
	 * @access	protected
	 */
	var $_db = null;
	
	/**
	 * Search index backend
	 * @var		object	AbstractIndex
	 * @access	protected
	 */
	var $_index = null;
	
	/**
	 * New document buffer
	 * @var		array
	 * @access	protected
	 */
	var $_documents = array();
	
	/**
	 * Document hashes
	 * @var		array
	 */
	var $_hashes = array();

	/**
	 * Number of chars to save from document
	 * @var		integer
	 * @access	protected
	 */
	var $_maxContentLength = 262144;
	
	/**
	 * Minimum revisit interval in seconds (default 24H)
	 * @var		integer
	 * @access	protected
	 */
	var $_minRevisitInterval = 86400;
	
	/**
	 * Default revisit interval in seconds (default 7D)
	 * @var		integer
	 * @access	protected
	 */
	var $_defaultRevisitInterval = 604800;
	
	/**
	 * Maximum revisit interval in seconds (default 30D)
	 * @var		integer
	 * @access	protected
	 */
	var $_maxRevisitInterval = 2592000;
	
	
	var $_docCount = null;
	var $_docLength = null;
	
	/**
	 * @param	Database		The database to store document contents
	 * @param	AbstractIndex	The index to use as search backend 
	 */
	function SyndWebIndex($db, $index) {
		$this->_index = $index;

		$index->attachHook('index_flush', array($this, '_callback_flush'));
		$index->attachHook('index_analyze', array($this, '_callback_index_analyze'));
		$index->attachHook('index_optimize', array($this, '_callback_index_optimize'));

		$index->attachHook('index_clear', array($this, '_callback_index_clear'));
		$index->attachHook('index_clear_section', array($this, '_callback_index_clear_section'));
		$index->attachHook('index_delete_document', array($this, '_callback_index_delete_document'));
		$index->attachHook('index_delete_batch', array($this, '_callback_index_delete_batch'));

		// Implements the 'site:' operator
		$index->attachHook('query_lexer_expression', array($this, '_callback_query_lexer_expression'));
		$index->attachHook('query_create_expression', array($this, '_callback_query_create_expression'));
		$index->attachHook('query_visit_term', array($this, '_callback_query_visit_term'), 30);

		if (function_exists('gzcompress'))
			$this->loadExtension(new SyndCompressExtension());
		
		if (is_object($db)) 
			$this->_db = $db;
		else if (SyndLib::isError($this->_db = DatabaseManager::getConnection($db))) {
			trigger_error($this->_db->getMessage(), E_USER_WARNING);
			$this->_db = DatabaseManager::getConnection('null://null');
		}
	}
	
	function loadExtension($extension) {
		$this->_index->loadExtension($extension);
	}
	
	function attachHook($name, $callback, $priority = 0) {
		$this->_index->attachHook($name, $callback, $priority);
	}
	
	function isBuffered($docid) {
		return isset($this->_documents[$docid]);
	}
	
	/**
	 * Returns a list of IIndexMatch matches
	 * @return	array
	 */
	function getMatchSet($query, $offset, $limit) {
		return $this->_index->getMatchSet($query, $offset, $limit);
	}
	
	function getMatchCount($query) {
		return $this->_index->getMatchCount($query);
	}

	function getExpandSet($query, $limit = 10) {
		return $this->_index->getExpandSet($query, $limit);
	}
	
	function getDocument($uri) {
		$sql = "
			SELECT w.* FROM synd_search_webpage w
			WHERE w.docid = ".crc32($uri);
		if (null != ($document = $this->_db->getRow($sql)))
			$this->_index->runHook('document_prepare_load', array($document));
		return $document;
	}
	
	function getDocuments($docids) {
		$sql = "
			SELECT w.* FROM synd_search_webpage w
			WHERE w.docid IN (".implode(',', $this->_db->quote($docids))."')";
		foreach (array_keys($documents = $this->_db->getAll($sql)) as $key)
			$this->_index->runHook('document_prepare_load', array($documents[$key]));
		return $documents;
	}
	
	function getSpellingSuggestions($terms) {
		return $this->_index->getSpellingSuggestions($terms);
	}

	function getDocumentCount() {
		$sql = "SELECT COUNT(*) FROM synd_search_webpage w";
		return $this->_db->getOne($sql);
	}

	function getSpiderQueueCount() {
		$sql = "SELECT COUNT(*) FROM synd_search_spider";
		return $this->_db->getOne($sql);
	}

	/**
	 * Adds the site: token to the lexer
	 */
	function _callback_query_lexer_expression(&$result) {
		$result[] = '\bsite\s*:\s*[\w.]+';
	}

	/**
	 * Instantiates the 'site:' token
	 */
	function _callback_query_create_expression(&$result, $token) {
		if (preg_match('/^site\s*:\s*([\w.]+)$/', $token, $matches)) {
			$result = new SyndSectionQuery('w.'.strrev($matches[1]));
			return true;
		}
	}

	/**
	 * Implements the 'site:' query operator
	 * @access	private
	 */
	function _callback_query_visit_term(&$result, $builder, $expression) {
		if (($expression instanceof SyndSectionQuery)) {
			$d = $builder->join('synd_search_document','d',array('docid'));
			$result = "$d.section LIKE ".$this->_index->_db->quote($expression->toString().'.%');
			return true;
		}
	}

	function getDocumentStructure($type, $uri, $response, &$title, &$content) {
		return $this->_index->runHook('extract', array($type, $uri, $response, &$title, &$content));
	}

	/**
	 * Adds a document to the index
	 * @param	string	The URI of this document
	 * @param	string	Title of document
	 * @param	string	Formatted document content
	 * @param	SyndDocument	Document content and structure
	 * @param	integer	Unextracted content length in bytes
	 * @param	integer	Last-Modified from header or extracted meta data
	 */
	function addDocument($uri, $title, $content, $document, $size, $modified = 0) {
		$parsed = parse_url($uri);
		$docid = crc32($uri);
		$hash = crc32($parsed['host'].$parsed['path']) ^ crc32($content);

		// Check for hash collisions
		if (!isset($this->_hashes[$hash]) ||
			strlen($uri) < 
			strlen($this->_documents[$this->_hashes[$hash]]['URI'])) {

			profiler_enter(__METHOD__);

			print "ADD $uri\n";
			if (isset($this->_hashes[$hash]))
				$this->_index->delDocument($this->_documents[$this->_hashes[$hash]]['URI']);
			
			// Calculate Time-To-Live for document
			if (0 != $modified && $modified < time() - 3600)
				$ttl = min(max(time()-$modified, $this->_minRevisitInterval), $this->_maxRevisitInterval);
			else {
				$ttl = $this->_defaultRevisitInterval;
				$modified = 0;
			}
			
			$this->_hashes[$hash] = $docid;
			$this->_documents[$docid] = array(
				'DOCID'		=> $docid,
				'HASH'		=> $hash,
				'MODIFIED'	=> $modified,
				'REVISIT'	=> time() + $ttl,
				'TTL'		=> $ttl,
				'SIZE'		=> $size,
				'FLAGS'		=> 0,
				'URI'		=> $uri,
				'TITLE'		=> $title,
				'CONTENT'	=> substr($content, 0, $this->_maxContentLength));
			
			$this->_index->runHook('document_prepare_store', 
				array($this->_documents[$docid]));

			$this->_documents[$docid]['CONTENT'] = $this->_documents[$docid]['CONTENT'];
			
			profiler_leave(__METHOD__);

			$this->_index->addDocument($uri, $document, 
				isset($parsed['host']) ? 'w.'.strrev($parsed['host']) : 'w');
				
			if (count($this->_documents) > 25)
				$this->flush();
		}
	}
	
	function delDocument($uri) {
		$this->_index->delDocument($uri);
	}
	
	function delDocumentBatch($uris) {
		$this->_index->delDocumentBatch($this->_index->documentId($uris));
	}
	
	function flush() {
		$this->_index->flush();
	}
	
	function commit() {
		$this->_index->commit();
	}
	
	function _callback_flush(&$result) {
		$delete = array();
		
		// Check for duplicates and recalculate TTL interval
		for ($i=0, $cnt=count($this->_documents); $i<$cnt; $i+=1000) {
			$sql = "
				SELECT w.DOCID, w.HASH, w.URI, w.TTL 
				FROM synd_search_webpage w
				WHERE 
					w.docid IN (".implode(', ', array_slice(array_keys($this->_documents), $i, 1000)).") OR
					w.hash IN (".implode(', ', array_slice(array_keys($this->_hashes), $i, 1000)).")";
			$documents = $this->_db->getAll($sql);
			
			foreach ($documents as $doc) {
				if (isset($this->_documents[$doc['DOCID']])) {
					// Decrese the time to live if document has changed
					if ($doc['HASH'] != $this->_documents[$doc['DOCID']]['HASH']) {
						$this->_documents[$doc['DOCID']]['TTL'] = 
							max($this->_minRevisitInterval, round($doc['TTL'] * 0.66));
						$this->_documents[$doc['DOCID']]['REVISIT'] = 
							time() + $this->_documents[$doc['DOCID']]['TTL'];
						$delete[] = $doc['DOCID'];
					}
					// Increase if not already at maximum
					else if ($this->_documents[$doc['DOCID']]['REVISIT'] < $this->_maxRevisitInterval) {
						$this->_documents[$doc['DOCID']]['TTL'] = 
							min($this->_maxRevisitInverval, round($doc['TTL'] * 1.33));
						$this->_documents[$doc['DOCID']]['REVISIT'] = 
							time() + $this->_documents[$doc['DOCID']]['TTL'];
						$delete[] = $doc['DOCID'];
					}
					// If already at max, don't reindex document at all
					else
						unset($this->_documents[$doc['DOCID']]);
				}
				else if (isset($this->_documents[$this->_hashes[$doc['HASH']]])) {
					// Dont reindex if same hash and longer uri
					if (strlen($this->_documents[$this->_hashes[$doc['HASH']]]['URI']) > strlen($doc['URI']))
						unset($this->_documents[$this->_hashes[$doc['HASH']]]);
					// Reindex if shorter uri
					else 
						$delete[] = $doc['DOCID'];
				}
			}
		}
		
		// Delete old documents
		if (!empty($delete))
			$this->_index->delDocumentBatch($delete);
		
		// Load new documents
		if (SyndLib::isError($result = $this->_db->load('synd_search_webpage', $this->_documents)))
			trigger_error($result->getMessage(), E_USER_ERROR);
		
		require_once 'core/lib/TemplateLib.inc';
		print "\nFlushed index ".date('Y-m-d H:i:s')." ".count($this->_documents)." documents";
		if (null != $this->_flushed)
			print ' in '.(time()-$this->_flushed);
		$this->_flushed = time();
		print "\n\n";

		// Deallocate memory
		unset($this->_documents, $this->_hashes);
		$this->_documents = array();
		$this->_hashes = array();
	}
	
	var $_flushed = null;

	function analyze() {
		$this->_index->analyze();
	}

	/**
	 * @access	private
	 */
	function _callback_index_analyze() {
		$this->_db->analyze('synd_search_webpage');
	}
	
	function optimize() {
		$this->_index->optimize();
	}
	
	/**
	 * @access	private
	 */
	function _callback_index_optimize() {
		$this->_db->optimize('synd_search_webpage');
	}

	/**
	 * @access	private
	 */
	function _callback_index_clear() {
		$this->_db->deleteQuick('synd_search_webpage', $where);
	}
	
	/**
	 * @access	private
	 */
	function _callback_index_clear_section(&$result, $section) {
		$where = "docid IN (
			SELECT d.docid FROM synd_search_document d
			WHERE d.section LIKE '$section.%')";
		$this->_db->deleteQuick('synd_search_webpage', $where);
	}

	/**
	 * @access	private
	 */
	function _callback_index_delete_document(&$result, $uri) {
		$docid = crc32($uri);
		$this->_db->deleteQuick('synd_search_webpage', "docid = $docid");
		if (isset($this->_documents[$docid])) {
			unset($this->_hashes[$this->_documents[$docid]['HASH']]);
			unset($this->_documents[$docid]);
		}
	}

	/**
	 * @access	private
	 */
	function _callback_index_delete_batch(&$result, $batch) {
		for ($i=0, $cnt=count($batch); $i<$cnt; $i+=1000) {
			$where = 'docid IN ('.implode(', ', array_slice($batch, $i, 1000)).')';
			$this->_db->deleteQuick('synd_search_webpage', $where);
		}
	}
}

/**
 * ZLIB compression of document text (synd_search_webpage.content)
 * @see		gzcompress()
 * @see		gzuncompress()
 * @link	http://www.faqs.org/rfcs/rfc1950
 */
class SyndCompressExtension extends SyndIndexExtension {
	function initialize($index) {
		$index->attachHook('document_prepare_store', array($this, '_callback_document_prepare_store'), 10);
		$index->attachHook('document_prepare_load', array($this, '_callback_document_prepare_load'), 10);
	}

	function _callback_document_prepare_store(&$result, $document) {
		if (strlen($document['CONTENT']) > 256) {
			$document['CONTENT'] = gzcompress($document['CONTENT'], 9);
			$document['FLAGS'] |= SYND_ENCODING_GZIP;
		}
	}

	function _callback_document_prepare_load(&$result, $document) {
		if ($document['FLAGS'] & SYND_ENCODING_GZIP)
			$document['CONTENT'] = gzuncompress($document['CONTENT']);
	}
}

/**
 * Persistent uri queue
 */
class SyndWebSpiderQueue extends ISyndSpiderQueue {
	var $_db = null;
	var $_index = null;
	
	/**
	 * @param	Database			The database to store the queue of uri's to be indexed
	 * @param	SyndWebIndex
	 */
	function SyndWebSpiderQueue($db, $index) {
		$this->_db = $db;
		$this->_index = $index;	
	}

	function addLocation($uri) {
		$this->addLocations(array($uri));
	}
	
	function addLocations($uris) {
		$docids = array_map('crc32', $uris);
		$keys = array_flip($docids);

		// Don't load already indexed documents into queue
		for ($i=0, $ids=$docids, $cnt=count($docids); $i<$cnt; $i+=1000) {
			$sql = "
				SELECT w.docid FROM synd_search_webpage w
				WHERE w.docid IN (".implode(', ', array_slice($ids, $i, 1000)).")
				UNION
				SELECT s.docid FROM synd_search_spider s
				WHERE s.docid IN (".implode(', ', array_slice($ids, $i, 1000)).")";
			$docids = array_diff($docids, $this->_db->getCol($sql, 0));
		}
		
		// Load URIs into queue
		$rows = array();
		foreach ($docids as $docid) {
			if (!$this->_index->isBuffered($docid)) {
				$rows[] = array(
					'DOCID' => $docid,
					'URI' => $uris[$keys[$docid]]);
			}
		}
		
		if (SyndLib::isError($result = $this->_db->load('synd_search_spider', $rows)))
			trigger_error($result->getMessage(), E_USER_WARNING);
		
		print "\nLoaded ".count($rows)." uris into database spider queue\n\n";
	}
	
	function getLocation() {
		return reset($this->getLocations(1));
	}
	
	function getLocations($limit) {
		$sql = "SELECT s.docid, s.uri FROM synd_search_spider s ORDER BY RAND()";
		$uris = $this->_db->getAssoc($sql, 0, $limit);
		
		// Delete documents from queue
		for ($i=0, $cnt=count($uris); $i<$cnt; $i+=1000) {
			$where = 'docid IN ('.implode(', ', array_slice(array_keys($uris), $i, 1000)).')';
			if (SyndLib::isError($result = $this->_db->deleteQuick('synd_search_spider', $where)))
				trigger_error($result->getMessage(), E_USER_ERROR);
		}
		
		// Fill up with documents due to be refreshed
		if (count($uris) < $limit) {
			$sql = "
				SELECT w.uri FROM synd_search_webpage w
				WHERE w.revisit < ".time()."
				ORDER BY w.revisit DESC";
			$uris = array_merge($uris, $this->_db->getCol($sql, 0, 0, $limit - count($uris)));
		}
		
		return $uris;
	}
}

/**
 * Restricts matches to a section in the index
 */
class SyndSectionQuery extends SyndTermQuery {
	function SyndSectionQuery($section) {
		parent::SyndTermQuery($section);
	}
	
	function getHeight() {
		return 0;
	}
}
