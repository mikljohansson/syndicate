<?php

/**
 * Index extension superclass
 * 
 * Use extension to dynamically add behavior to an index. Behaviour
 * could for example include html filtering, weighting schemes or
 * different index strategies.
 *
 * @access		public
 * @package		synd.core.index
 */
class SyndIndexExtension {
	/**
	 * Index we're attached to
	 * @var		object	AbstractIndex
	 * @access	protected
	 */
	var $_index = null;
	
	function __sleep() {
		$vars = array_keys(get_object_vars($this));
		unset($vars[array_search('_index', $vars)]);
		return $vars;
	}

	/**
	 * Attaches this extension to an index.
	 * @param	AbstractIndex	Index to attach to
	 */
	function initialize($index) {
		$this->_index = $index;
	}
	
	function loadGene($gene)  {}
	function getDefaultGene() {return array();}
	function getMinimumGene() {return array();}
	function getMaximumGene() {return array();}
}
