<?php
require_once 'core/model/type/collection.class.inc';
require_once 'core/lib/rpc/RpcTransport.class.inc';

class synd_type_indexed_collection extends synd_type_collection {
	var $_urns = null;
	var $_query = null;
	var $_sections = null;
	var $_attributes = null;
	var $_rset = null;
	var $_operator = null;
	var $_order = null;
	
	/**
	 * Cached total match count
	 * @access	private
	 */
	var $_count = null;
	
	/**
	 * Cached counts per section
	 * @access	private
	 */
	var $_counts = null;

	/**
	 * Cached contents per section
	 * @access	private
	 */
	var $_contents = null;
	
	function __construct($urns, $query, $sections, $attributes, $rset = null, $operator = '&&', $order = 'date') {
		parent::__construct();
		$this->_urns = $urns;
		$this->_query = $query;
		$this->_sections = $sections;
		$this->_attributes = $attributes;
		$this->_rset = $rset;
		$this->_operator = $operator;
		$this->_order = $order;
	}
	
	function __sleep() {
		$vars = array_keys(get_object_vars($this));
		unset($vars[array_search('_count', $vars)]);
		unset($vars[array_search('_counts', $vars)]);
		unset($vars[array_search('_contents', $vars)]);
		return $vars;
	}
	
	function toString() {
		return $this->_query;
	}
	
	function getCount() {
		if (null === $this->_count) {
			foreach ($this->_urns as $urn) {
				if (null !== ($backend = Activator::getInstance($urn)) && false !== ($this->_count = 
					$backend->getMatchCount($this->_query, $this->_sections, $this->_attributes, $this->_operator)))
					break;
			}
		}
		return $this->_count;
	}
	
	function getContents($offset = 0, $limit = null) {
		$mset = null;
		$result = array();

		foreach ($this->_urns as $urn) {
			if (null !== ($backend = Activator::getInstance($urn)) && false !== ($mset = 
				$backend->getMatchSet($this->_query, $this->_sections, $this->_attributes, $offset, $limit, $this->_operator, $this->_rset, true, $this->_order)))
				break;
		}
		
		if (null !== $mset) {
			foreach ($mset[0] as $match) {
				if (0 === strpos($match['SECTION'], 'n.'))
					SyndNodeLib::preload($match['PAGEID']);
			}

			foreach ($mset[0] as $match) {
				if (0 === strpos($match['SECTION'], 'n.') && null !== ($node = SyndLib::getInstance($match['PAGEID'])))
					$result[$node->id()] = $node;
			}

			$this->_count = $mset[1];
		}

		return $result;
	}

	function getFilteredCount($classes) {
		if (empty($classes))
			return $this->getCount();
		if (!count($sections = $this->_getSections($classes)))
			return 0;
		
		$hash = SyndLib::hash($sections);
		if (!isset($this->_counts[$hash])) {
			foreach ($this->_urns as $urn) {
				if (null !== ($backend = Activator::getInstance($urn)) && false !== ($this->_counts[$hash] = 
					$backend->getMatchCount($this->_query, $this->_sections, $this->_attributes, $this->_operator)))
					break;
			}
		}
		
		return $this->_counts[$hash];
	}
	
	function getFilteredContents($classes) {
		if (empty($classes))
			return $this->getContents();
		if (!count($sections = $this->_getSections($classes)))
			return array();
		
		$hash = SyndLib::hash($sections);
		if (!isset($this->_contents[$hash])) {
			$mset = null;
			
			foreach ($this->_urns as $urn) {
				if (null !== ($backend = Activator::getInstance($urn)) && false !== ($mset = 
					$backend->getMatchCount($this->_query, $this->_sections, $this->_attributes, $this->_operator)))
					break;
			}

			if (null !== $mset) {
				foreach ($mset[0] as $match) {
					if (0 === strpos($match['SECTION'], 'n.'))
						SyndNodeLib::preload($match['PAGEID']);
				}

				foreach ($mset[0] as $match) {
					if (0 === strpos($match['SECTION'], 'n.') && null !== ($node = SyndLib::getInstance($match['PAGEID'])))
						$this->_contents[$hash][$node->id()] = $node;
				}

				$this->_counts[$hash] = $mset[1];
			}
		}
		
		return $this->_contents[$hash];
	}
	
	/**
	 * @access	private
	 */
	function _getSections($classes) {
		$sections = array();
		foreach ($classes as $class) {
			if (0 === strpos($class, 'synd_node_'))
				$sections[] = 'n.'.SyndNodeLib::getInheritedBranch(substr($class, 10));
		}
		return array_intersect($sections, (array)$this->_sections);
	}
}

