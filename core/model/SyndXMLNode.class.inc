<?php

/**
 * XML DOM node
 *
 * Methods employe null pattern meaning they always return an object
 * satisfying the SyndXMLNode interface. Use the isNull() method to 
 * check for null objects.
 *
 * @access		public
 * @package		synd.core.model
 */
class SyndXMLNode {
	/**
	 * Element name
	 * @access	protected
	 */
	var $_name = null;

	/**
	 * CDATA text content
	 * @access	protected
	 */
	var $_content = null;

	/**
	 * Attribute hashmap
	 * @access	protected
	 */
	var $_attributes = null;

	/**
	 * 
	 * @access	protected
	 */
	var $_children = null;

	/**
	 * @param	string	Tagname
	 * @param	string	Text/CDATA content
	 */
	function __construct($name, $content = null) {
		$this->_name = $name;
		$this->_content = (string)$content;
	}

	/**
	 * Returns the default null instance
	 * @return	SyndXMLNode
	 */
	function nullInstance() {
		return SyndXMLNull::instance();
	}

	function __clone() {
		foreach (array_keys((array)$this->_children) as $key)
			$this->_children[$key] = clone $this->_children[$key];
	}
	
	/**
	 * Tag name
	 * @return	string
	 */
	function getName() {
		return $this->_name;
	}

	/**
	 * Text content
	 * @return	string
	 */
	function getContent() {
		return $this->_content;
	}
	
	/**
	 * Renders this node into a string
	 * @param	string	Private recursion parameter
	 * @return	string
	 */
	function toString($_indent = '') {
		if (count($children = $this->getChildren())) {
			$xml = $_indent.'<'.$this->_name.$this->_implode().'>';
			foreach (array_keys($children) as $key) {
				$xml .= "\r\n";
				$xml .= $children[$key]->toString($_indent.'    ');
			}
			$xml .= "\r\n".$_indent.'</'.$this->_name.'>';
		} 
		else if (null != ($content = $this->getContent())) { 
			$xml = $_indent.'<'.$this->_name.$this->_implode().'>';
			$xml .= $content;
			$xml .= '</'.$this->_name.'>';
		} else { 
			$xml = $_indent.'<'.$this->_name.$this->_implode().'/>';
		}
		return $xml;
	}

	/**
	 * @access	private
	 */
	function _implode() {
		if (!empty($this->_attributes))
			return ' '.tpl_html_implode($this->_attributes);
		return null;
	}
	
	/**
	 * Set the text content
	 * @param	string	Text/CDATA content
	 */
	function setContent($content) {
		$this->_content = trim($content);
	}
	
	/**
	 * Appends a child node
	 * @param	synd_syncml_node
	 * @return	synd_syncml_node
	 */
	function appendChild($child) {
		assert('is_object($child)');
		if (!$child->isNull())
			$this->_children[] = $child;
		return $child;
	}
	
	/**
	 * Removes al child nodes
	 */
	function removeChildren() {
		$this->_children = null;
	}
	
	/**
	 * Removes the last added child node
	 */
	function removeLastChild() {
		if (!empty($this->_children))
			array_pop($this->_children);
	}
	
	/**
	 * Null object pattern
	 * @return	bool
	 */
	function isNull() {
		return false;
	}

	/**
	 * Returns all child nodes
	 * @return	array
	 */
	function getChildren() {
		return (array)$this->_children;
	}
	
	/**
	 * Returns the first child node
	 * @return	synd_syncml_node
	 */
	function getFirstChild() {
		if (count($children = $this->getChildren()))
			return $children[reset(array_keys($children))];
		return $this->nullInstance();
	}

	/**
	 * Returns the value of an attribute on this node
	 * @param	string	Attribute name
	 * @return	string
	 */
	function getAttribute($name) {
		return isset($this->_attributes[$name]) ? $this->_attributes[$name] : null;
	}

	/**
	 * Set an attribute on this node
	 * @param	string	Attribute name
	 * @param	string	Attribute value
	 * @return	string
	 */
	function setAttribute($name, $value) {
		$this->_attributes[$name] = $value;
	}
	
	/**
	 * Returns the first child node by tagname
	 * @param	string	The tagname to look for
	 * @return	synd_syncml_node
	 */
	function getElement($name) {
		foreach (array_keys($children = $this->getChildren()) as $key) {
			if ($name == $children[$key]->_name)
				return $children[$key];
		}
		return $this->nullInstance();
	}
	
	/**
	 * Returns all child nodes having a tagname
	 * @param	string	The tagname to search for
	 * @return	array
	 */
	function getElements($name) {
		$elements = array();
		foreach (array_keys($children = $this->getChildren()) as $key) {
			if ($name == $children[$key]->_name)
				$elements[] = $children[$key];
		}
		return $elements;
	}
	
	/**
	 * Selects the first node mathing a path
	 *
	 * <code>
	 *  $cred = $document->selectSingleNode('SyncML', 'SyncHdr', 'Cred');
	 * </code>
	 *
	 * @return	synd_syncml_node
	 */
	function selectSingleNode() {
		$node = $this;
		foreach (func_get_args() as $name) {
			$node = $node->getElement($name);
			if ($node->isNull()) break;
		}
		return $node;
	}
	
	/**
	 * Selects all nodes mathing a path
	 *
	 * <code>
	 *  $statusNodes = $document->selectNodes('SyncML', 'SyncBody', 'Status');
	 * </code>
	 *
	 * @return	synd_syncml_node
	 */
	function selectNodes() {
		if (1 == count($names = func_get_args()))
			return $this->getElements(array_shift($names));
		$nodes = array();
		foreach (array_keys($children = $this->getElements(array_shift($names))) as $key) {
			$nodes = SyndLib::array_merge_assoc($nodes, 
				call_user_func_array(array($children[$key], 'selectNodes'), $names));
		}
		return $nodes;
	}

	/**
	 * Parses an XML document
	 * @param	string		XML to parse
	 * @return	SyndXMLNode
	 */
	static function parse($xml) {
		$document = new SyndXMLNode(null);
		$document->_parse($xml);
		return $document;
	}
	
	/**
	 * Parses an XML document into this node
	 * @param	string	XML to parse
	 */
	function _parse($xml) {
		$parser = xml_parser_create('ISO-8859-1');
		xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, false);
		xml_parser_set_option($parser, XML_OPTION_SKIP_WHITE, true);
		
		xml_set_element_handler($parser, 
			array($this, '_callback_enter'),
			array($this, '_callback_leave'));
		xml_set_character_data_handler($parser, 
			array($this, '_callback_cdata'));

		$this->_stack[] = $this;
		xml_parse($parser, $xml, true);

		$this->_stack = null;
		xml_parser_free($parser);
	}
	
	/**
	 * @access	private
	 */
	function _callback_enter($parser, $name, $attributes) {
		if (null === $this->_name) {
			$node = $this;
			$this->_name = $name;
		}
		else {
			$node = new SyndXMLNode($name);
			$current = $this->_stack[count($this->_stack)-1];
			$current->appendChild($node);
		}

		foreach ($attributes as $key => $value)
			$node->setAttribute($key, $value);
		$this->_stack[] = $node;
	}
	
	/**
	 * @access	private
	 */
	function _callback_leave($parser, $name) {
		$current = $this->_stack[count($this->_stack)-1];
		if (null != $current->_content)
			$current->_content = trim($current->_content);
		array_pop($this->_stack);
	}

	/**
	 * @access	private
	 */
	function _callback_cdata($parser, $cdata) {
		$current = $this->_stack[count($this->_stack)-1];
		if ('' != ($content = trim($cdata)) || null != $current->_content)
			$current->_content .= "\n" == $cdata ? "\r\n" : $cdata;
	}
}

/**
 * Null instance
 */
class SyndXMLNull extends SyndXMLNode {
	function __construct() {
		parent::__construct(null);
	}
	
	static function instance() {
		return new self();
	}
	
	function isNull() {
		return true;
	}
	
	function copy($name = null) {
		return $this;
	}
	
	function appendChild($child) {
		return $child;
	}
	
	function setContent($content) {}
	function setAttribute($name, $value) {}
}
