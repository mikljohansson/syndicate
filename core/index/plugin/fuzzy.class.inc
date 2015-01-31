<?php
require_once 'core/i18n.inc';
require_once 'core/index/SyndIndexExtension.class.inc';

/**
 * Fuzzy searching extension
 * 
 * Enables searching on for example metaphone or soundex keys. This 
 * might improve precision by finding misspelled words and similar
 * sounding matches.
 *
 * metaphone is tuned specifically for english but will work for
 * other western languages. double_metaphone is an improvement
 * on the regular metaphone and can handle foreign words better. 
 *
 * This extension should not be used together with MySQL 4.1 because
 * of the need for index_merge operations which are only available
 * in MySQL 5+, other backends such as Oracle doesn't have this 
 * problem and the extension should work fine.
 *
 * @todo		Use a self-join to get around the one index per table limit
 *
 * @see			metaphone()
 * @package		synd.core.index
 */
class synd_plugin_fuzzy extends SyndIndexExtension {
	/**
	 * String length in bytes
	 *
	 * You have to change the database schema when changing this.
	 * double_metaphone() and soundex() uses 4 bytes while the 
	 * regular metaphone() varies in length
	 *
	 * @var		string
	 * @access	protected
	 */
	var $_length = 8;

	/**
	 * Callback function that implements the algorithm.
	 * @var		string
	 * @access	protected
	 */
	var $_callback = 'metaphone';
	
	/**
	 * @access	private
	 */
	var $_stack = array(false);

	/**
	 * Takes the callback function to use.
	 *
	 * Callback function should take one argument, the word to process
	 * and return the processed string.
	 *
	 * @param	mixed	Callback function
	 */
	function __construct($callback = 'metaphone', $length = 8) {
		$this->_callback = $callback;
		$this->_length = $length;
	}
	
	function initialize($index) {
		parent::initialize($index);
		$index->attachHook('index_term', array($this, '_callback_index_term'));

		$index->attachHook('query_lexer_expression', array($this, '_callback_query_lexer_expression'));
		$index->attachHook('query_create_expression', array($this, '_callback_query_create_expression'));

		$index->attachHook('query_visit_term', array($this, '_callback_query_visit_term'), 5);
		$index->attachHook('query_visit_null', array($this, '_callback_query_visit_null'), 5);
		$index->attachHook('query_spelling_suggestions', array($this, '_callback_query_spelling_suggestions'));
	}
	
	/**
	 * Adds the ~ token to the lexer
	 */
	function _callback_query_lexer_expression(&$result) {
		$result[] = '~';
	}

	/**
	 * Instantiates the ~ token
	 */
	function _callback_query_create_expression(&$result, $token) {
		if ('~' == $token) {
			$result = new SyndFuzzyOperator($this);
			return true;
		}
	}

	function enterFuzzyBranch() {
		array_push($this->_stack, true);
	}
	
	function leaveFuzzyBranch() {
		array_pop($this->_stack);
	}
	
	function _callback_query_visit_term(&$result, $builder, $expression) {
		if (end($this->_stack)) {
			$t = $builder->allocate('synd_search_termindex','t',array('docid','termid'));
			$td = $builder->allocate('synd_search_term','td',array('termid'));

			$term = $expression->toString();
			$fuzzy = substr(call_user_func($this->_callback, $term), 0, $this->_length);

			// Note: MySQL 4.1 misoptimizes this query, version 5+ uses index_merge
			$result = "($td.term LIKE '$term%' OR $td.fuzzy = '$fuzzy')";
			return true;
		}
	}

	function _callback_query_visit_null(&$result, $builder, $expression) {
		if (end($this->_stack))
			$td = $builder->allocate('synd_search_term','td',array('termid'));
	}
	
	/**
	 * Adds the fuzzy value
	 */
	function _callback_index_term($result, &$data) {
		profiler_enter(__METHOD__);
		$data['FUZZY'] = "'".$this->_fuzzy($data['TERM'])."'";
		profiler_leave(__METHOD__);
	}

	/**
	 * Suggests terms with similar spelling but higher frequency
	 */
	function _callback_query_spelling_suggestions(&$result, $terms) {
		$processed = array_map(array($this->_index, 'termProcess'), 
			array_map('synd_strtolower', $terms));
		
		$N = $this->_index->getDocumentCount();
		$sql = "
			SELECT td.term, LN(($N-td.n+0.5) / (td.n+0.5)) FROM synd_search_term td
			WHERE td.term IN (".implode(', ', $this->_index->_db->quote($processed)).")";
		$weights = $this->_index->_db->getAssoc($sql);
		
		// Check for rare or nonexistent terms
		foreach ($processed as $i => $term) {
			if ((!isset($weights[$term]) || $weights[$term] > 8) && !preg_match('/\d/', $term)) {
				// Find terms with same metaphone key but lower weight == higher frequency
				$sql = "
					SELECT td.TERM, td.ORIGINAL, LN(($N-td.n+0.5) / (td.n+0.5)) WEIGHT
					FROM synd_search_term td
					WHERE 
						td.term != ".$this->_index->_db->quote($term)." AND
						td.fuzzy = ".$this->_index->_db->quote($this->_fuzzy($term));
				if (isset($weights[$term]))
					$sql .= " AND LN(($N-td.n+0.5) / (td.n+0.5)) < {$weights[$term]}";
				$sql .= "
					ORDER BY LN(($N-td.n+0.5) / (td.n+0.5))";
				$suggestions = $this->_index->_db->getAll($sql, 0, 10);
					
				// Sort by levenshtein distance since metaphone isn't very exact
				if (!empty($suggestions)) {
					$ratings = array();
					foreach ($suggestions as $suggestion) 
						$ratings[$suggestion['ORIGINAL']] = levenshtein($suggestion['TERM'], $term) * $suggestion['WEIGHT'];
					
					asort($ratings);
					$result[$terms[$i]] = reset(array_keys($ratings));
				}
			}
		}
	}

	function _fuzzy($term) {
		return substr(call_user_func($this->_callback, $term), 0, $this->_length);
	}
}

/**
 * Fuzzy match operator.
 * @package		synd.core.index
 */
class SyndFuzzyOperator extends SyndUnaryOperator {
	var $_extension = null;

	function SyndFuzzyOperator($extension, $rhs = null) {
		parent::SyndUnaryOperator($rhs);
		$this->_extension = $extension;
	}
	
	function getPrecedence() {
		return 40;
	}
	
	function isBounded() {
		return null != $this->_rhs && $this->_rhs->isBounded();
	}

	function acceptVisitor($visitor) {
		$this->_extension->enterFuzzyBranch();
		$this->_rhs->acceptVisitor($visitor);
		$this->_extension->leaveFuzzyBranch();
	}

	function isLeftAssociative() {
		return true;
	}
}
