<?php

/**
 * The index builder is used to modify the index
 * @access		public
 * @package		synd.core.index
 */
interface IndexBuilder {
	/**
	 * Open a new document
	 * @param	string	The id used to identify the document
	 * @param	string	Section to index under
	 * @param	array	Attributes to add to document
	 * @param	integer	The timestamp the document was last modified
	 */
	function open($pageid, $section = null, $attributes = array(), $modified = 0);
	
	/**
	 * Close the open document
	 */
	function close();

	/**
	 * Flush documents to storage
	 */
	function flush();

	/**
	 * Extract and add file contents to document
	 * @param	string	Path or uri of file
	 * @param	string	Content type
	 * @param	string	Optional file contents
	 */
	function visitFile($file, $type = null, $buffer = null);
	
	/**
	 * Parse and add string contents to document
	 * @param	string	Document fragment to be added
	 * @param	string	Field name this fragment belongs to {title, url, ..}
	 * @param	float	Fragment weight (0 <= weight <= 1)
	 */
	function visitFragment($fragment, $field = null, $weight = 1);

	/**
	 * Add single term to this document
	 * @param	string	Term to add
	 * @param	string	Field name this term belongs to {title, url, ..}
	 */
	function visitTerm($term, $field = null);
}

class ForwardingIndexBuilder implements IndexBuilder {
	protected $_next = null;
	
	function __construct(IndexBuilder $next) {
		$this->_next = $next;
	}
	
	function open($pageid, $section = null, $attributes = array(), $modified = 0) {
		$this->_next->open($pageid, $section, $attributes, $modified);
	}

	function close() {
		$this->_next->close();
	}
	
	function flush() {
		$this->_next->flush();
	}
	
	function visitFile($file, $type = null, $buffer = null) {
		$this->_next->visitFile($path, $type, $buffer);
	}
	
	function visitFragment($fragment, $field = null, $weight = 1) {
		$this->_next->visitFragment($fragment, $field, $weight);
	}

	function visitTerm($term, $field = null) {
		$this->_next->visitTerm($term, $field);
	}
}
