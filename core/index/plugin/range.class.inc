<?php
require_once 'core/defines.inc';
require_once 'core/index/SyndIndexExtension.class.inc';
require_once 'core/index/SyndExpression.class.inc';

/**
 * Number ranges extension (query like "1..500")
 */
class synd_plugin_range extends SyndIndexExtension {
	var $_numerics = array();
	
	function initialize($index) {
		parent::initialize($index);
		$index->attachHook('query_lexer_expression', array($this, '_callback_query_lexer_expression'));
		$index->attachHook('query_create_expression', array($this, '_callback_query_create_expression'));
		$index->attachHook('query_visit_operator', array($this, '_callback_query_visit_operator'));
		$index->attachHook('query_branched_operator', array($this, '_callback_query_branched_operator'));

		$index->attachHook('document_fragment', array($this, '_callback_document_fragment'));
		$index->attachHook('index_posting', array($this, '_callback_index_posting'), 10);

		$index->attachHook('index_flush', array($this, '_callback_index_flush'));
		$index->attachHook('index_analyze', array($this, '_callback_index_analyze'));
		$index->attachHook('index_optimize', array($this, '_callback_index_optimize'));

		$index->attachHook('index_clear', array($this, '_callback_index_clear'));
		$index->attachHook('index_clear_section', array($this, '_callback_index_clear_section'));
		$index->attachHook('index_delete_document', array($this, '_callback_index_delete_document'));
		$index->attachHook('index_delete_batch', array($this, '_callback_index_delete_batch'));
	}

	function getNumericCount() {
		$sql = "SELECT COUNT(*) FROM synd_search_numeric";
		return $this->_index->_db->getOne($sql);
	}

	/**
	 * Adds the '..' token to the lexer
	 */
	function _callback_query_lexer_expression(&$result) {
		$result[] = '\.{2,3}';
		$result[] = '\d[\d\-\/:]*(?=\.\.)';
		$result[] = '(?<=\.\.)\d[\d\-\/:]*';
	}

	/**
	 * Instantiates the '..' token
	 */
	function _callback_query_create_expression(&$result, $token) {
		if ('..' == $token || '...' == $token) {
			$result = new SyndRangeQuery();
			return true;
		}
	}
	
	/**
	 * Match for ISO 8601, MM/DD/YY and other dates and index them
	 */
	function _callback_document_fragment(&$result, $document) {
		profiler_enter(__METHOD__);

		$content = $document->toString();
		if (preg_match_all('/\b\d{2}((?:\d{2})?[\-:]\d{2}[\-:]\d{2}(?=\b|T)|\/\d{2}\/(?:\d{2}|\d{4})\b)/i', 
			strlen($content) < 32768 ? $content : substr($content, 0, 32768), $matches)) {

			$docid = $this->_index->getCurrentDocumentId();
			$wdw = $this->_index->getWithinDocumentWeight($document);
			
			foreach ($matches[0] as $date) {
				if (-1 != ($timestamp = strtotime(strtr($date, ':', '-')))) {
					$this->_index->runHook('index_posting', 
						array($docid, date('Ymd', $timestamp), 0, 0, $wdw));
				}
			}
		}

		profiler_leave(__METHOD__);
	}

	/**
	 * Add numbers to the numeric index
	 */
	function _callback_index_posting($result, $docid, &$term, $field, $context, $wdw, $nonChunked = null) {
		profiler_enter(__METHOD__);

		// Don't index chunks from splitting long numbers (PhraseExtension::_callback_index_posting),
		// but update the termid to join on the first chunk instead of whole number
		if (null != $nonChunked) {
			if ($this->_index->termId($nonChunked) == $this->_numerics[$docid.'.'.$nonChunked]['termid'])
				$this->_numerics[$docid.'.'.$nonChunked]['termid'] = $this->_index->termId($term);
		}
		else if (is_numeric($term)) {
			$this->_numerics[$docid.'.'.$term] = array(
				'termid' => $this->_index->termId($term),
				'docid' => $docid,
				'term' => $term);
		}

		profiler_leave(__METHOD__);
	}
	
	/**
	 * @access	private
	 */
	function _parse($string) {
		if (preg_match('/\b\d{2}((?:\d{2})?[\-:]\d{2}[\-:]\d{2}(?=\b|T)|\/\d{2}\/(?:\d{2}|\d{4})\b)/i', $string, $matches) &&
			-1 != ($timestamp = strtotime(strtr($matches[0], ':', '-').(isset($matches[1])?'':'-01'))))
			return date('Ymd', $timestamp);
		return preg_replace("/[^{$SYND_WORDCHARS}]/", '', $string);
	}
	
	function _callback_query_visit_operator(&$result, $builder, $expression) {
		if (($expression instanceof SyndRangeQuery)) {
			$lhs = $this->_parse($expression->_lhs->toString());
			$rhs = $this->_parse($expression->_rhs->toString());
			if (!is_numeric($lhs) && !is_numeric($rhs))
				return;

			$t = $builder->allocate('synd_search_termindex','t',array('docid','termid'));
			$tn = $builder->allocate('synd_search_numeric','tn',array('docid','termid'));
			
			if (is_numeric($lhs) && is_numeric($rhs))
				$result = "$tn.term BETWEEN $lhs AND $rhs";
			else if (is_numeric($lhs))
				$result = "$tn.term > $lhs";
			else if (is_numeric($rhs))
				$result = "$tn.term > $rhs";

			return true;
		}
	}

	/**
	 * @access	private
	 */
	function _callback_index_flush() {
		if (SyndLib::isError($result = $this->_index->_db->load('synd_search_numeric', $this->_numerics)))
			trigger_error($result->getMessage(), E_USER_WARNING);

		// Deallocate memory properly
		unset($this->_numerics);
		$this->_numerics = array();
	}
	
	/**
	 * @access	private
	 */
	function _callback_index_analyze() {
		$this->_index->_db->analyze('synd_search_numeric');
	}
	
	/**
	 * @access	private
	 */
	function _callback_index_optimize() {
		$this->_index->_db->optimize('synd_search_numeric');
	}
	
	/**
	 * @access	private
	 */
	function _callback_index_clear() {
		$this->_index->_db->truncate('synd_search_numeric');
	}
	
	/**
	 * @access	private
	 */
	function _callback_index_clear_section(&$result, $section) {
		$where = "docid IN (
			SELECT d.docid FROM synd_search_document d
			WHERE d.section LIKE '$section.%')";
		$this->_index->_db->deleteQuick('synd_search_numeric', $where);
	}

	/**
	 * @access	private
	 */
	function _callback_index_delete_document(&$result, $pageId) {
		$where = "docid = ".$this->_index->documentId($pageId);
		$this->_index->_db->deleteQuick('synd_search_numeric', $where);
	}

	/**
	 * @access	private
	 */
	function _callback_index_delete_batch(&$result, $batch) {
		for ($i=0, $cnt=count($batch); $i<$cnt; $i+=1000) {
			$where = 'docid IN ('.implode(', ', array_slice($batch, $i, 1000)).')';
			$this->_index->_db->deleteQuick('synd_search_numeric', $where);
		}
	}

	function _callback_query_branched_operator(&$result, $visitor, $expression) {
		if (($expression instanceof SyndRangeQuery) && $expression->isBounded()) {
			$result++;
			return true;
		}
	}
}

class SyndRangeQuery extends SyndBooleanAND {
	function SyndRangeQuery($lhs = null, $rhs = null) {
		parent::SyndBooleanAND($lhs, $rhs);
	}

	function getPrecedence() {
		return 37;
	}
	
	function getHeight() {
		return $this->isBounded() ? 1 : parent::getHeight();
	}

	function isBounded() {
		$lhs = null !== $this->_lhs ? SyndRangeExtension::_parse($this->_lhs->toString()) : null;
		$rhs = null !== $this->_rhs ? SyndRangeExtension::_parse($this->_rhs->toString()) : null;
		return is_numeric($lhs) && is_numeric($rhs);
	}

	function reduceExpression(&$context) {
		if (($i = count($context['operands'])) < 2)
			return;
		
		if (is_numeric(SyndRangeExtension::_parse($context['operands'][$i-1]->toString())) &&
			is_numeric(SyndRangeExtension::_parse($context['operands'][$i-2]->toString())))
			parent::reduceExpression($context);
		else {
			// Parse as AND if non numeric arguments
			$rhs = SyndLib::array_pop($context['operands']);
			$lhs = SyndLib::array_pop($context['operands']);
			$context['operands'][] = new SyndBooleanAND($lhs, $rhs);
		}
	}
}
