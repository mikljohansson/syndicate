<?php

/**
 * Represents a document that can be indexed.
 * 
 * @access		public
 * @package		synd.core.index
 */
class SyndDocument {
	/**
	 * @var		string
	 * @access	private
	 */
	var $_field = null;

	/**
	 * @var		float
	 * @access	private
	 */
	var $_weight = 1;
	
	/**
	 * Last modifed timestamp
	 * @var		integer
	 * @access	private
	 */
	var $_modified = 0;
	
	/**
	 * @param	string	Field this document belongs to. {title,keywords,body,...}
	 * @param	float	Weight of this document (0 <= weight <= 1)
	 */
	function SyndDocument($field = null, $weight = 1, $modified = 0) {
		$this->_field = $field;
		$this->_weight = $weight;
		$this->_modified = $modified;
	}

	/**
	 * Returns a string representation of this document.
	 * @return	string
	 */
	function toString() {}

	/**
	 * Returns the field this document belongs to
	 * @return	string
	 */
	function getField() {
		return $this->_field;
	}
	
	/**
	 * Returns the weight of this document (0 <= weight <= 1)
	 * @return	float
	 */
	function getWeight() {
		return $this->_weight;
	}

	/**
	 * Last modified timestamp
	 * @return	integer
	 */
	function getModified() {
		return $this->_modified;
	}

	/**
	 * Set last modified timestamp
	 * @param	integer	Unix timestamp
	 */
	function setModified($modified) {
		$this->_modified = $modified;
	}

	/**
	 * Accepts a visitor.
	 *
	 * Child classes will override this to call specific
	 * methods on the visitor.
	 *
	 * <code>
	 * Visitor must implement:
	 *   void visitDocumentFragment(SyndDocumentFragment)
	 *   void enterDocumentComposite(SyndDocumentComposite)
	 *   void leaveDocumentComposite(SyndDocumentComposite)
	 * </code>
	 *
	 * @param	SyndDocumentVisitor
	 */
	function acceptVisitor($visitor) {}
}

/**
 * Document text fragment.
 *
 * Use this to represent the pure text fragments of the document.
 */
class SyndDocumentFragment extends SyndDocument {
	/**
	 * @var		string
	 * @access	private
	 */
	var $_text = null;

	function SyndDocumentFragment($text, $field = null, $weight = 1, $modified = 0) {
		parent::SyndDocument($field, $weight, $modified);
		$this->_text = $text;
	}
	
	function toString() {
		return $this->_text;
	}
	
	function setText($text) {
		$this->_text = $text;
	}
	
	function acceptVisitor($visitor) {
		$visitor->visitDocumentFragment($this);
	}
}

/**
 * Document composite.
 *
 * Use this to represent multiple fragments of a document, children
 * can themselves be composites allowing for tree structures.
 */
class SyndDocumentComposite extends SyndDocument {
	/**
	 * @var		array
	 * @access	private
	 */
	var $_fragments = array();
	
	function SyndDocumentComposite($fragments, $field = null, $weight = 1, $modified = 0) {
		parent::SyndDocument($field, $weight, $modified);
		$this->_fragments = $fragments;
	}
	
	function toString() {
		$string = null;
		foreach (array_keys($this->_fragments) as $key) {
			if (null != $this->_fragments[$key])
				$string .= ' '.$this->_fragments[$key]->toString();
		}
		return trim($string);
	}

	/**
	 * Will recurse children.
	 */
	function acceptVisitor($visitor) {
		$visitor->enterDocumentComposite($this);
		foreach (array_keys($this->_fragments) as $key) {
			if (null != $this->_fragments[$key])
				$this->_fragments[$key]->acceptVisitor($visitor);
		}
		$visitor->leaveDocumentComposite($this);
	}
}
/**
 * Document page.
 *
 * Represent a webpage or other document containing links to other
 * documents, these will be used by LinkdAnalysisWeightingScheme.
 */
class SyndDocumentPage extends SyndDocumentComposite {
	var $_links = null;
	
	function SyndDocumentPage($fragments, $links = null) {
		parent::SyndDocumentComposite($fragments);
		$this->_links = $links;
	}
	
	function getLinks() {
		return (array)$this->_links;
	}
	
	function setLinks($links) {
		$this->_links = $links;
	}

	function acceptVisitor($visitor) {
		$visitor->visitDocumentPage($this);
		foreach (array_keys($this->_fragments) as $key) {
			if (null != $this->_fragments[$key])
				$this->_fragments[$key]->acceptVisitor($visitor);
		}
	}
}
