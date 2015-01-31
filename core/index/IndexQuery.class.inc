<?php
require_once 'core/i18n.inc';

/**
 * Search query parser
 *
 * Builds an expression tree from a query like
 *  '(regulation import) OR export +"canned fish" -japan'
 *
 * @access		public
 * @package		synd.core.index
 */
class IndexQuery {
	private $_text 			= null;
	private $_sections		= null;
	private $_attributes	= null;
	private $_rset			= null;
	private $_operator		= null;
	private $_order			= array();
	private $_index			= null;
	private $_matches		= array();
	
	/**
	 * @param	string	Query string
	 * @param	array	Index sections to query
	 * @param	array	Security attributes to query
	 * @param	array	Ids of documents deemed relevant
	 * @param	string	Implicit operator to use
	 */
	function __construct($text = null, $sections = null, $attributes = null, $rset = null, $operator = '&&') {
		$this->_text = $text;
		$this->_sections = $sections;
		$this->_attributes = $attributes;
		$this->_rset = $rset;
		$this->_operator = $operator;
	}
	
	/**
	 * Returns the query text.
	 * @return	string
	 */
	function toString($index) {
		return $index->stripQueryText($this->_text);
	}

	function setMatchCount($hash, $matches) {
		$this->_matches[$hash] = $matches;
	}
	
	function getMatchCount($hash) {
		return isset($this->_matches[$hash]) ? $this->_matches[$hash] : null;
	}

	/**
	 * Returns the index sections to query.
	 * @return	array
	 */
	function getSections() {
		return $this->_sections;
	}
	
	/**
	 * Returns the security contexts to query.
	 * @return	array
	 */
	function getSecurityAttributes() {
		return $this->_attributes;
	}

	/**
	 * Returns the documents deemed relevant to the query
	 * @return	array
	 */
	function getRelevanceSet() {
		return (array)$this->_rset;
	}
	
	function getImplicitOperator() {
		return $this->_operator;
	}
	
	/**
	 * Normalized Query Length (nql), the number of terms in the query.
	 * @return	integer
	 */
	function getLength($index) {
		preg_match_all("/([{$SYND_WORDCHARS}]+)/", $this->toString($index), $matches);
		return ($c = count(array_unique($matches[1]))) ? $c : count($this->_rset);
	}

	/**
	 * Order the query results
	 *
	 * If the FieldExtension is loaded it will order the queries
	 * accoring to the fields sent to this method.
	 *
	 * @param	string	Field to order on
	 * @param	bool	Sort ascending or descending
	 */
	function order($field, $asc = true) {
		$this->_order[] = $field;
		$this->_order[] = $asc;
	}
	
	/**
	 * Returns the order fields for this query
	 * @return	array
	 */
	function getOrderBy() {
		return $this->_order;
	}

	/**
	 * Read a token from stream and instantiate an expression
	 * @param	array	Token stream
	 * @return	SyndExpression	null on end of stream
	 */
	function readExpression(&$stream) {
		if (false === (list(, $token) = each($stream)) || ')' == $token)
			$result = null;
		// Parse and wrap a subexpression
		else if ('(' == $token)
			$result = new SyndExpressionWrapper($this->buildExpression($stream));
		// Request index/extensions to instantiate expression
		else
			$result = $this->_index->createExpression($token);
		
		return $result;
	}
	
	/**
	 * Returns the default expression.
	 * @return	SyndExpression
	 */
	function defaultExpression() {
		return $this->_index->createExpression($this->_operator);
	}

	/**
	 * Builds an expression tree from a token stream.
	 * @param	array	Stream of tokens
	 * @return	SyndExpression
	 */
	function buildExpression(&$stream) {
		$context = array(
			'operators' => array(),
			'operands' => array());

		if (null == ($state = $this->readExpression($stream)))
			return null;

		while (null != ($state = $state->nextExpression($this, $stream, $context)));
		while (null != ($state = SyndLib::array_pop($context['operators'])))
			$state->reduceExpression($context);

		return end($context['operands']);
	}

	/**
	 * Returns the query parse tree.
	 * @param	AbstractIndex	The index implementation
	 * @return	SyndExpression
	 */
	function createExpression($index) {
		static $lambda = null;
		$this->_index = $index;
		
		// Tokenize operators, words and phrases
        $lexer = implode('|', $index->lexerExpression());
        preg_match_all("/$lexer/i", $this->toString($index), $matches);
		$stream = $matches[0];
		
		// Strip empty tokens
		if (null == $lambda)
			$lambda = create_function('&$a', '$a=trim($a," \r\n\t");');
		array_walk($stream, $lambda);
		$stream = array_values(array_filter($stream, 'strlen'));
		reset($stream);

		// Build tree
		return $this->buildExpression($stream);
	}
}
