<?php
require_once 'core/index/SyndIndexExtension.class.inc';

/**
 * Link analysis weighting scheme (PageRank)
 * @package		synd.core.index
 */
class synd_plugin_pagerank extends SyndIndexExtension {
	var $_links = array();
	
	/**
	 * The pagerank constant
	 * @var		float
	 * @access	protected
	 */
	var $_K1 = 0.999984;
	
	/**
	 * Maximum rank (bit width of the RANK column)
	 * @var		integer
	 * @access	protected
	 */
	var $_maxRank = 0xFFFF;
	
	/**
	 * @var		array
	 * @access	private
	 */
	var $_sections = array();
	
	function initialize($index) {
		parent::initialize($index);
		$index->attachHook('document_page', array($this, '_callback_document_page'));
		$index->attachHook('index_flush', array($this, '_callback_index_flush'));
		$index->attachHook('query', array($this, '_callback_query'));
	}
	
	function getLinkCount() {
		$sql = "SELECT COUNT(*) FROM synd_search_link l";
		return $this->_index->_db->getOne($sql);
	}

	function _callback_document_page(&$result, $document) {
		$links = $document->getLinks();
		
		$source = $this->_index->getCurrentDocumentId();
		$this->_sections[$this->_index->getCurrentSection()][] = $source;
		$this->_index->_documents[$source]['links'] = min(255, max(1, count($links)));

		foreach ($links as $uri) {
			$this->_links[] = array(
				'TARGET' => crc32($uri),
				'SOURCE' => $source);
		}
	}

	/**
	 * Adjusts pagerank of documents to be flushed
	 */
	function _callback_index_flush() {
		if (SyndLib::isError($result = $this->_index->_db->load('synd_search_link', $this->_links)))
			return trigger_error($result->getMessage(), E_USER_ERROR);

		unset($this->_links);
		$this->_links = array();

		if (!$this->_index->inTransaction()) {
			foreach ($this->_sections as $section => $docids) {
				for ($i=0, $cnt=count($docids); $i<$cnt; $i+=1000) {
					$sql = "
						SELECT l.target, SUM(d.rank / d.links)
						FROM synd_search_link l, synd_search_document d
						WHERE 
							l.source = d.docid AND
							l.target IN (".implode(', ', array_slice($docids, $i, 1000)).") AND
							d.section != ".$this->_index->_db->quote($section)."
						GROUP BY l.target";
					$ranks = $this->_index->_db->getAssoc($sql);
					
					foreach ($ranks as $docid => $rank) {
						if (isset($this->_index->_documents[$docid]))
							$this->_index->_documents[$docid]['rank'] = min($this->_maxRank,  ceil($this->_maxRank * (1 - pow($this->_K1, $rank+1))));
					}
				}
			}
		}
	}

	/**
	 * Builds a query.
	 * @param	SqlQueryBuilder	The query builder
	 * @param	IndexQuery			The query to run
	 * @param	SyndExpression		The expression to filter on
	 * @return	string
	 */
	function _callback_query(&$result, $builder, $query, $expression) {
		$d = $builder->join('synd_search_document','d',array('docid'));
		$builder->column("1 + $d.rank / $this->_maxRank", 'WEIGHT', '*');
		$builder->order('WEIGHT', false);
	}
}

//DROP TABLE IF EXISTS synd_search_link_temp;
//CREATE TABLE synd_search_link_temp (
// TARGET int(11),
// RANK int(11),
// PRIMARY KEY TARGET (TARGET)) TYPE=MyISAM;
//
//INSERT INTO synd_search_link_temp
//SELECT l.target, 65535 * (1 - POWER(0.999984, SUM(d0.rank / d0.links)+1))
//FROM synd_search_link l, synd_search_document d0, synd_search_document d1
//WHERE
//	l.source = d0.docid AND
//	l.target = d1.docid AND
//	d0.section != d1.section
//GROUP BY l.target;
//
//UPDATE synd_search_document
//SET rank = (
//    SELECT t2.rank FROM synd_search_link_temp t2
//    WHERE t2.target = synd_search_document.docid)
//WHERE EXISTS (
//    SELECT 1 FROM synd_search_link_temp t
//    WHERE t.target = synd_search_document.docid);
//
//DROP TABLE synd_search_link_temp;
//
