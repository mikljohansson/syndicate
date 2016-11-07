<?php
require_once 'core/i18n.inc';
require_once 'core/index/SyndEventProducer.class.inc';
require_once 'core/index/Index.class.inc';
require_once 'core/lib/SyndLib.class.inc';

/**
 * Index interface/superclass.
 *
 * @abstract
 * @access		public
 * @package		synd.core.index
 */
abstract class AbstractIndex extends SyndEventProducer implements Index {
	const PRIO_BUILDER_SINK		= 750;
	const PRIO_BUILDER_TERM		= 500;
	const PRIO_BUILDER_FRAGMENT	= 250;
	const PRIO_BUILDER_FILE		= 0;
	
	/**
	 * @access	private
	 */
	var $_inTransaction = false;
	
	/**
	 * @access	private
	 */
	var $_exitHandler = false;
	
	/**
	 * Terms will be chopped at this length
	 *
	 * This value must correspond to the width of the 
	 * synd_search_term.term column in the database.
	 *
	 * @access	private
	 */
	var $_maxTermLength = 32;
	
	/**
	 * Constructor
	 * @param	Database to use
	 */
	function __construct() {
		$this->attachHook('text_strip_query', array($this, '_callback_text_strip_query'), -100);
		$this->attachHook('builder', array($this, '_callback_fragment_parser'), self::PRIO_BUILDER_FRAGMENT);
	}

	function _callback_fragment_parser(&$result) {
		$result = new synd_parser_preg($result);
	}		
	
	function getExpandSet(IndexQuery $query, $limit = 10) {
		return array();
	}

	/**
	 * Checks for alternate spellings
	 *
	 * <pre>
	 * Returns a list like 
	 *	array(
	 *		'speling' => 'spelling', 
	 *		'errror' => 'error')
	 * </pre>
	 *
	 * @param	array	Terms to check
	 * @return	array
	 */
	function getSpellingSuggestions($terms) {
		return (array)$this->runHook('query_spelling_suggestions', array($terms));
	}

	/**
	 * Returns the docid given an id
	 * @param	mixed	Id of page or array of id's
	 * @return	mixed
	 */
	function documentId($pageid) {
		return $pageid;
	}
	
	function getIndexer() {
		return $this->runHook('builder');
	}
	
	/**
	 * Deletes a document from the index
	 * @param	string	Id of document
	 */
	function delDocument($pageid) {
		if (false == $this->_exitHandler) {
			SyndLib::attachHook('shutdown', array($this, 'flush'));
			$this->_exitHandler = true;
		}
		$this->runHook('index_delete_document', array($pageid));
	}

	/**
	 * Deletes a batch of documents
	 * @access	protected
	 * @param	array	Array of docid's
	 */
	function delDocumentBatch($batch) {
		if (false == $this->_exitHandler) {
			SyndLib::attachHook('shutdown', array($this, 'flush'));
			$this->_exitHandler = true;
		}
		$this->runHook('index_delete_batch', array($batch));
	}

	function inTransaction() {
		return $this->_inTransaction;
	}

	/**
	 * Takes the index offline (remember to do commit())
	 *
	 * If a transaction is not started the index will attempt to keep
	 * its internal state consistent at all times and inserts will 
	 * therefore be somewhat slower. 
	 *
	 * Work is instead done on commit so don't forget to finish the 
	 * transaction with commit() or the index will be left in an 
	 * inconsitent state (can be fixed with a call to commit() later).
	 * Use transactions when batch-indexing/spidering and leave the 
	 * index online when inserting only a few documents.
	 */
	function begin() {
		if (false == $this->_inTransaction)
			$this->_inTransaction = true;
	}

	/**
	 * Commits the transaction
	 */
	function commit() {
		$this->runHook('index_commit');
	}

	/**
	 * Perform index maintenance
	 */
	function analyze() {
		$this->runHook('index_analyze');
	}
	
	/**
	 * Perform (very costly) index maintenance
	 */
	function optimize() {
		$this->runHook('index_optimize');
	}

	/**
	 * Returns the number of indexed documents
	 * @param	string	The section to count
	 * @return	integer
	 */
	function getDocumentCount($section = null) {
		return 0;
	}

	/**
	 * Clears the entire index, use with caution.
	 */
	function clear() {
		$this->runHook('index_clear');
	}
	
	/**
	 * Clears a section of the index
	 *
	 * This can potentially be a very costly operation depending on 
	 * the number of distinct terms. The clear() function however; is
	 * very efficient since it just truncates the whole index.
	 *
	 * @param	string	Section to clear
	 */
	function clearSection($section) {
		$this->runHook('index_clear_section', array($section));
	}

	/**
	 * Prepare query for parsing.
	 * @param	string	Text to prepare
	 * @return	string
	 */
	function stripQueryText($text) {
		return $this->runHook('text_strip_query', array(&$text));
	}

	function _callback_text_strip_query(&$result, &$text) {
		$result = $text;
		return true;
	}
	
	function lexerExpression() {
		global $SYND_WORDCHARS, $SYND_TOKENIZE_GLUE;
		$word = $SYND_WORDCHARS;
		$glue = $SYND_TOKENIZE_GLUE;

		return array_merge((array)$this->runHook('query_lexer_expression'), array(
				'[()]',
				'[&|]{1,2}',
				"(?<![$word+\-!])[+\-!]",
				"[$word]([$word]|[$glue](?=[$word]))*"));
	}

	function createExpression($token) {
		if (null === ($expression = $this->runHook('query_create_expression', array($token)))) {
			switch ($token) {
				case '+':
					$expression = new SyndBooleanInclude();
					break;

				case '-':
				case '!':
					$expression = new SyndBooleanExclude();
					break;

				case '&':
				case '&&':
					$expression = new SyndBooleanAND();
					break;

				case '|':
				case '||':
					$expression = new SyndBooleanOR();
					break;
					
				default: 
					$expression = new SyndTermQuery($token);
			}
		}
		return $expression;
	}
	
	function termProcess($term) {
		$this->runHook('query_process_term', array(&$term));
		return $term;
	}
}
