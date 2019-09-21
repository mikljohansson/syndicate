<?php
require_once 'core/index/SyndIndexExtension.class.inc';
require_once 'core/index/SyndExpression.class.inc';

/**
 * Field mathing extension
 * 
 * Enables searching in specific fields in term indexes. Stores the
 * signature of the field for each term posting. If possible, use 
 * the following standard fields when indexing documents: 
 *
 * <pre>
 *  id			An unambiguous reference to the resource within a given context.
 *  title		A name given to the resource.
 *	creator		An entity primarily responsible for making the content of the resource.
 *  description	An account of the content of the resource.
 *  subject		A topic or keywords of the content of the resource.
 *  created		Date of creation of the resource.
 *  body		Full resource text.
 * </pre>
 *
 * @see			SyndPhraseExtension
 * @package		synd.core.index
 */
class synd_plugin_field extends SyndIndexExtension {
	/**
	 * Length of signature in bits
	 * @var		integer
	 * @access	private
	 */
	var $_length = 8;
	
	/**
	 * Bits used by each hash key
	 * @var		integer
	 * @access	private
	 */
	var $_bits = 3;
	
	/**
	 * Field hashes
	 * @var		array
	 * @access	private
	 */
	var $_hashes = array(
		'body' 			=> null,
		'title' 		=> null,
		'creator'		=> null,
		'description' 	=> null,
		'subject' 		=> null,
		'created' 		=> null,
		'id'			=> 'title',
		'filetype'		=> null,
		'type'			=> 'filetype',
	);
	
	/**
	 * Field value buffer
	 * @var		array
	 * @access	private
	 */
	var $_buffer = array();
	
	/**
	 * Max field content length in bytes (for sorting)
	 * @var		integer
	 * @access	private
	 */
	var $_sortLength = 10;
	
	/**
	 * Enable sorting on fields
	 * @var		bool
	 * @access	private
	 */
	var $_sorting = false;
	
	/**
	 * Field stack when adding document
	 * @var		array
	 * @access	private
	 */
	var $_stack = array();
	
	/**
	 * Last field belonged to a fragment
	 * @access	private
	 */
	var $_isFragment = false;
	
	function __construct($sorting = false) {
		$this->_sorting = $sorting;
	}

	function initialize($index) {
		parent::initialize($index);
		
		$index->attachHook('index_posting', array($this, '_callback_index_posting'), 20);

		$index->attachHook('query_lexer_expression', array($this, '_callback_query_lexer_expression'));
		$index->attachHook('query_create_expression', array($this, '_callback_query_create_expression'));
		$index->attachHook('query_visit_operator', array($this, '_callback_query_visit_operator'));
		
		$index->attachHook('document_fragment', array($this, '_callback_document_fragment'));
		$index->attachHook('document_composite_enter', array($this, '_callback_document_composite_enter'));
		$index->attachHook('document_composite_leave', array($this, '_callback_document_composite_leave'));

		if ($this->_sorting) {
			$index->attachHook('query', array($this, '_callback_query'), 10);

			$index->attachHook('index_flush', array($this, '_callback_index_flush'));
			$index->attachHook('index_analyze', array($this, '_callback_index_analyze'));
			$index->attachHook('index_optimize', array($this, '_callback_index_optimize'));

			$index->attachHook('index_clear', array($this, '_callback_index_clear'));
			$index->attachHook('index_clear_section', array($this, '_callback_index_clear_section'));
			$index->attachHook('index_delete_document', array($this, '_callback_index_delete_document'));
			$index->attachHook('index_delete_batch', array($this, '_callback_index_delete_batch'));
		}
	}
	
	function getSortingCount() {
		$sql = "SELECT COUNT(*) FROM synd_search_field";
		return $this->_index->_db->getOne($sql);
	}

	/**
	 * Add a new field to be recognized
	 * @param	string	Name of field
	 */
	function addField($field) {
		$this->_hashes[$field] = null;
	}
	
	/**
	 * Add a new field to be recognized as an alias of another field
	 * @param	string	Name of field
	 * @param	string	Alias
	 */
	function addFieldAlias($field, $alias) {
		$this->_hashes[$alias] = $field;
	}

	function getFields() {
		return $this->_hashes;
	}
	
	/**
	 * Adds the : token to the lexer
	 */
	function _callback_query_lexer_expression(&$result) {
		$result[] = ':';
	}

	/**
	 * Instantiates the ':' token
	 */
	function _callback_query_create_expression(&$result, $token) {
		if (':' == $token) {
			$result = new SyndFieldQuery();
			return true;
		}
	}

	/**
	 * Calculates a hash for a value.
	 *
	 * The hash will be $this->length bits long and contain 
	 * $this->_bits number of 1's.
	 *
	 * @access	protected
	 * @param	string	Field to hash
	 * @return	integer
	 */
	function _hash($field) {
		if (null == $field)
			return 0;
		profiler_enter(__METHOD__);
		
		if (!isset($this->_hashes[$field])) {
			$this->_hashes[$field] = 0;
			for ($i=0, $v=strtolower($field); $i<$this->_bits; $i++)
				$this->_hashes[$field] |= 1 << abs(crc32(md5($i.$v.$i))) % $this->_length;
		}
		else if (is_string($this->_hashes[$field]))
			$this->_hashes[$field] = $this->_hash($this->_hashes[$field]);
		
		profiler_leave(__METHOD__);
		return $this->_hashes[$field];
	}
	
	/**
	 * Implements the field query operator
	 * @access	private
	 */
	function _callback_query_visit_operator(&$result, $builder, $expression) {
		if (($expression instanceof SyndFieldQuery) && null !== $expression->_rhs) {
			// If the field name is not registered, parse _rhs like a normal query
			if (null === $expression->_lhs || !array_key_exists($expression->_lhs->toString(), $this->_hashes))
				$expression->_rhs->acceptVisitor($builder);
			else {
				$builder->enterExpression();
				$builder->pushContext();

				$hash = $this->_hash($expression->_lhs->toString());
				for ($i=$expression->_rhs->getHeight(), $j=0; $i > 0; $i--, $j--) {
					if ($j)
						$builder->_sql .= ' AND ';
					$t = $builder->allocate('synd_search_termindex','t',array('docid','termid'));
					$builder->_sql .= $this->_index->_db->bitand("$t.field", $hash)." = $hash";
				}
				
				$builder->popContext();
				
				$builder->_sql .= ' AND ';
				$expression->_rhs->acceptVisitor($builder);

				$builder->leaveExpression();
			}

			$result = '';
			return true;
		}
	}

	/**
	 * Calculates the field hash for a posting.
	 */
	function _callback_index_posting($result, $docid, $term, &$field) {
		profiler_enter(__METHOD__);
		$field = $this->_hash(reset($this->_stack));
		profiler_leave(__METHOD__);
	}
	
	/**
	 * Implements order-by-field queries
	 * @access	private
	 */
	function _callback_query(&$result, $builder, $query, $expression) {
		if (count($order = $query->getOrderBy())) {
			foreach ($order as $i => $field) {
				if (is_string($field)) {
					$df = $builder->allocate('synd_search_field','df',array('docid'));
					$asc = !isset($order[$i+1]) || false !== $order[$i+1];
					$hash = $this->_hash($field);
					$builder->where("$df.field = $hash");
					$builder->order("$df.content", $asc);
				}
			}
		}
	}

	/**
	 * Pushes document field and stores the text if sorting is enabled
	 * @access	private
	 */
	function _callback_document_fragment(&$result, $document) {
		if ($this->_isFragment) {
			$this->_isFragment = false;
			array_pop($this->_stack);
		}
		
		if (null != $document->getField()) {
			array_push($this->_stack, $document->getField());
			$this->_isFragment = true;
		}
		
		if ($this->_sorting && null != ($field = reset($this->_stack))) {
			$id = $this->_index->_context['docid'].'.'.$field;
			if (isset($this->_buffer[$id])) {
				$content = trim($this->_buffer[$id]['content'],"'");
				$content .= ' '.$document->toString();
				$this->_buffer[$id]['content'] = substr(trim($content),0,$this->_sortLength);
			}
			else {
				$this->_buffer[$id] = array(
					'field' => $this->_hash($field),
					'docid' => $this->_index->_context['docid'],
					'content' => substr(trim($document->toString()),0,$this->_sortLength));
			}
		}
	}
	
	/**
	 * Pushes the document field to the stack
	 * @access	private
	 */
	function _callback_document_composite_enter(&$result, $document) {
		if ($this->_isFragment) {
			$this->_isFragment = false;
			array_pop($this->_stack);
		}
		if (null != $document->getField())
			array_push($this->_stack, $document->getField());
	}
	
	/**
	 * Pops the document field from the stack
	 * @access	private
	 */
	function _callback_document_composite_leave(&$result, $document) {
		if (null != $document->getField())
			array_pop($this->_stack);
	}
	
	/**
	 * @access	private
	 */
	function _callback_index_flush() {
		$this->_index->_db->load('synd_search_field', $this->_buffer);
		unset($this->_buffer);
		$this->_buffer = array();
	}
	
	/**
	 * @access	private
	 */
	function _callback_index_analyze() {
		$this->_index->_db->analyze('synd_search_field');
	}
	
	/**
	 * @access	private
	 */
	function _callback_index_optimize() {
		$this->_index->_db->optimize('synd_search_field');
	}
	
	/**
	 * @access	private
	 */
	function _callback_index_clear() {
		$this->_index->_db->truncate('synd_search_field');
	}
	
	/**
	 * @access	private
	 */
	function _callback_index_clear_section(&$result, $section) {
		$where = "docid IN (
			SELECT d.docid FROM synd_search_document d
			WHERE d.section LIKE '$section.%')";
		$this->_index->_db->deleteQuick('synd_search_field', $where);
	}

	/**
	 * @access	private
	 */
	function _callback_index_delete_document(&$result, $pageId) {
		$sql = "
			DELETE FROM synd_search_field
			WHERE docid = ".$this->_index->documentId($pageId);
		$this->_index->_db->query($sql);
	}

	/**
	 * @access	private
	 */
	function _callback_index_delete_batch(&$result, $batch) {
		for ($i=0, $cnt=count($batch); $i<$cnt; $i+=1000) {
			$where = 'docid IN ('.implode(', ', array_slice($batch, $i, 1000)).')';
			$this->_index->_db->deleteQuick('synd_search_field', $where);
		}
	}
}

/**
 * Phrase expression.
 * @package		synd.core.index
 */
class SyndFieldQuery extends SyndBooleanAND {
	var $_extension = null;

	function getHeight() {
		return $this->_rhs->getHeight();
	}
	
	function isBounded() {
		return null != $this->_lhs && !in_array($this->_lhs->toString(), array('filetype','type')) &&
			null != $this->_rhs && $this->_rhs->isBounded();
	}

	function getPrecedence() {
		return 33;
	}
	
	function getField() {
		return null !== $this->_lhs ? $this->_lhs->toString() : null;
	}
	
	function acceptVisitor($visitor) {
		$visitor->visitUnaryOperator($this, $this->_rhs);
	}
}

