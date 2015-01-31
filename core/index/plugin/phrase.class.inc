<?php
require_once 'core/i18n.inc';
require_once 'core/index/SyndIndexExtension.class.inc';
require_once 'core/index/SyndExpression.class.inc';

/**
 * Phrase searching extension
 * 
 * Enables phrase searching on term indexes. Stores the signature
 * of the term context for each term posting. The principle is
 * the same as for 'signature files'. 
 *
 * For each term posting the signature of the following term is
 * computed (3 way hash) and OR into the context signature for that
 * term posting. Searching involves computing the signature of the 
 * next term in the phrase and checking (via AND) if the context 
 * matches.
 * 
 * This extension will split long (strlen > 4) numbers into pieces
 * on 'visit_document_fragment'. When querying for long numbers the
 * term will be replaced with a phrase query for the pieces on
 * 'query_visit_term'. The purspose is to not fill up the term
 * list with serial numbers.
 *
 * @package		synd.core.index
 */
class synd_plugin_phrase extends SyndIndexExtension {
	/**
	 * Length of signature in bits
	 * @var		integer
	 * @access	protected
	 */
	var $_length = 16;
	
	/**
	 * Bits used by each hash key
	 * @var		integer
	 * @access	protected
	 */
	var $_bits = 4;
	
	/**
	 * Caches term hashes
	 * @var		array
	 * @access	private
	 */
	var $_hashes = array();
	
	var $_prev = 0;
	var $_prevSecond = 0;

	function initialize($index) {
		parent::initialize($index);
		$index->attachHook('index_posting', array($this, '_callback_index_posting'));
		$index->attachHook('document_fragment', array($this, '_callback_document_fragment'));

		$index->attachHook('query_lexer_expression', array($this, '_callback_query_lexer_expression'));
		$index->attachHook('query_create_expression', array($this, '_callback_query_create_expression'));

		$index->attachHook('query_visit_term', array($this, '_callback_query_visit_term'), 15);
		$index->attachHook('query_branched_term', array($this, '_callback_query_branched_term'), 15);
	}

	/**
	 * Adds the " token to the lexer
	 */
	function _callback_query_lexer_expression(&$result) {
		global $SYND_WORDCHARS;
		$result[] = '"';
		$result[] = "[{$SYND_WORDCHARS}]+(?:[\-_\/\\\\]|(?<!\d)\d|\d(?!\d))[{$SYND_WORDCHARS}][{$SYND_WORDCHARS}\-_\/\\\\]*";
		$result[] = '\b([\da-f]{2}:)+[\da-f]{2}\b'; // MAC addresses
	}

	/**
	 * Instantiates the " token
	 */
	function _callback_query_create_expression(&$result, $token) {
		global $SYND_WORDCHARS;
		if ('"' == $token) {
			$result = new SyndPhraseQuery();
			return true;
		}
		
		if (preg_match("/^[{$SYND_WORDCHARS}]+(?:[\-_\/\\\\]|(?<!\d)\d|\d(?!\d))[{$SYND_WORDCHARS}][{$SYND_WORDCHARS}\-_\/\\\\]*$/", $token) ||
			preg_match('/^([\da-f]{2}:)+[\da-f]{2}$/', $token)) {
			$result = new SyndPhraseQuery($token);
			return true;
		}
	}

	/**
	 * Calculates a hash for a value.
	 *
	 * The hash will be $this->length bits long and contain 
	 * $this->_bits number of 1's.
	 *
	 * @param	string	Value to hash
	 * @return	integer
	 */
	function _hash($value) {
		profiler_enter(__METHOD__);
		
		if (!isset($this->_hashes[$value])) {
			if (count($this->_hashes) > 5000)
				array_splice($this->_hashes, 0, 1000);
			$this->_hashes[$value] = 0;
			for ($i=0; $i<$this->_bits; $i++)
				$this->_hashes[$value] |= (1 << (abs(crc32(md5($i.$value.$i))) % $this->_length));
		}

		profiler_leave(__METHOD__);
		return $this->_hashes[$value];
	}
	
	function _callback_document_fragment() {
		$this->_prevSecond = 0;
		$this->_prev = 0;
	}
	
	/**
	 * Calculates the context for a posting
	 */
	function _callback_index_posting($result, $docid, &$term, &$field, &$context, $wdw) {
		profiler_enter(__METHOD__);

		if (strlen($term) > 4 && is_numeric($term)) {
			for ($i=0, $length=strlen($term); $i<$length; $i+=4) {
				$context = 0;
				$this->_index->runHook('index_posting', 
					array($docid, substr($term,$i,4), &$field, &$context, $wdw, $term));
			}

			profiler_leave(__METHOD__);
			return true;
		}
		else {
			$context = $this->_prev | ($this->_prevSecond >> 1);
			$this->_prevSecond = $this->_prev;
			$this->_prev = $this->_hash($term);
		}
		
		profiler_leave(__METHOD__);
	}
	
	/**
	 * Implements the phrase query in sql
	 */
	function _callback_query_visit_term(&$result, $builder, $expression) {
		if (($expression instanceof SyndPhraseQuery)) {
			$terms = array();
			$filter = array();
			
			foreach ($expression->getTerms() as $term) {
				if (is_numeric($term) && strlen($term) > 4)
					$terms = array_merge($terms, explode(' ', wordwrap($expression->toString(), 4, ' ', true)));
				else
					$terms[] = $term;
			}
			
			for ($i=0; $i<count($terms); $i++) {
				// Add regular term filters
				$filter[] = $this->_index->runHook('query_visit_term', array($builder, new SyndTermQuery($terms[$i])));
				$t = $builder->join('synd_search_termindex','t',array('docid','termid'));

				// Context filter for next two terms
				if ($i >= 1) {
					$hash = $this->_hash($terms[$i-1]);
					if ($i >= 2)
						$hash |= ($this->_hash($terms[$i-2]) >> 1);
					
					$filter[] = $this->_index->_db->bitand("$t.context", $hash)." = $hash";
				}
			}
			
			$result = implode(' AND ', $filter);
			return true;
		}
		else if (is_numeric($expression->toString()) && strlen($expression->toString()) > 4) {
			$result = $this->_index->runHook('query_visit_term', array($builder,
				new SyndPhraseQuery(wordwrap($expression->toString(), 4, ' ', true))));
			return true;
		}
	}

	function _callback_query_branched_term(&$result, $visitor, $expression) {
		if (($expression instanceof SyndPhraseQuery)) {
			foreach ($expression->getTerms() as $term) {
				$query = new SyndTermQuery($term);
				$query->acceptVisitor($visitor);
			}
			return true;
		}
	}
}

/**
 * Phrase expression.
 * @package		synd.core.index
 */
class SyndPhraseQuery extends SyndTermQuery {
	/**
	 * @var		array
	 * @access	protected
	 */
	var $_terms = array();
	
	var $_final = false;

	function SyndPhraseQuery($phrase = null) {
		if (null != $phrase) {
			$this->_token($phrase);
			$this->_final = true;
		}
	}
	
	function getHeight() {
		return count($this->_terms);
	}
	
	function toString() {
		return implode(' ', $this->_terms);
	}
	
	function setString($string) {
		$this->_terms = explode(' ', $string);
	}

	function getTerms() {
		return $this->_terms;
	}
	
	function reduceExpression(&$context) {
		if (count($this->_terms))
			parent::reduceExpression($context);
	}

	function nextExpression($query, &$stream, &$context) {
		if (!$this->_final) {
			// Consume all terms
			$firstKey = key($stream);
			while (null != (list($key, $token) = each($stream)) && '"' != $token) 
				$this->_token($token);

			// No terms in phrase or no end delimiter
			if (count($this->_terms) < 1 || '"' != $token) {
				SyndLib::array_set_current($stream, $firstKey);
				if ('"' == $token)
					unset($stream[$key]);
				$this->_terms = array();
			}
			
			$this->_final = true;
		}
		return parent::nextExpression($query, $stream, $context);
	}
	
	function _token($token) {
		global $SYND_WORDCHARS;
		foreach (array_filter(preg_split("/[^{$SYND_WORDCHARS}]+|(?<=\d)(?!\d)|(?<!\d)(?=\d)/", $token, -1), 'strlen') as $term) {
			if (is_numeric($term) && strlen($term) > 4)
				$this->_terms = array_merge($this->_terms, explode(' ',wordwrap($term,4,' ',true)));
			else 
				$this->_terms[] = $term;
		}
	}
}
