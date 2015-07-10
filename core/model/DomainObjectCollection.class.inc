<?php

/**
 * Collection of domain objects
 * @access		public
 * @package		synd.core.model
 */
interface DomainObjectCollection extends IteratorAggregate, Countable {
	/**
	 * Iterates over this collection
	 * @param	int		Offset to start iteration at
	 * @param	int		The number of objects to iterate
	 * @param	array	Columns to sort on, eg. array('TS_CREATE', false, 'INFO_HEAD')
	 * @return	Iterator
	 */
	//abstract function getIterator($offset = 0, $limit = null, $order = null);
}

/**
 * Overlays a number of append or remove operations onto another collection
 */
abstract class BufferedDomainObjectCollection implements ArrayAccess, DomainObjectCollection {
	protected $_append = null;
	protected $_remove = null;
	protected $_parent = null;
	
	function __construct(DomainObject $parent) {
		$this->_parent = $parent;
	}
	
	function getIterator($offset = 0, $limit = null, $order = null) {
		if (empty($this->_append) && empty($this->_remove))
			return $this->_getIterator($offset, $limit, $order);
		
		if (!$offset) {
			return new BufferedDomainObjectIterator(
				$this->_getIterator($offset, $limit, $order), 
				SyndNodeLib::getInstances(array_keys((array)$this->_append)),
				SyndNodeLib::getInstances(array_keys((array)$this->_remove)),
				$offset, $limit, $order);
		}

		return new BufferedDomainObjectIterator(
			$this->_getIterator(max($offset-count($this->_append), 0), $limit+count($this->_append), $order), 
				SyndNodeLib::getInstances(array_keys((array)$this->_append)),
				SyndNodeLib::getInstances(array_keys((array)$this->_remove)),
			$offset, $limit, $order);
	}

	function count() {
		return $this->_count() + count($this->_append) - count($this->_remove);
	}
	
	/**
	 * Returns the contained iterator
	 * @return	Iterator
	 */
	abstract function _getIterator($offset = 0, $limit = null, $order = null);

	/**
	 * Returns the contained count
	 * @return	int
	 */
	abstract function _count();

	/**
	 * Appends an entity from this collection
	 * @param	DomainObject	The object to append
	 * @return	DomainObject
	 */
	function append(DomainObject $node) {
		if (isset($this->_remove[$node->nodeId]))
			unset($this->_remove[$node->nodeId]);
		$this->_append[$node->nodeId] = true;
		return $node;
	}
	
	/**
	 * Removes an entity from this collection
	 * @param	DomainObject	The object to remove
	 * @return	DomainObject
	 */
	function remove(DomainObject $node) {
		if (isset($this->_append[$node->nodeId]))
			unset($this->_append[$node->nodeId]);
		else
			$this->_remove[$node->nodeId] = true;
		return $node;
	}
	
	/**
	 * Removes an entity from this collection
	 * @param	DomainObject	The object to remove
	 * @return	DomainObject
	 */
	function removeTemporary(DomainObject $node) {
		if (isset($this->_append[$node->nodeId]))
			unset($this->_append[$node->nodeId]);
		else
			$this->_remove[$node->nodeId] = true;
		return $node;
	}

	/**
	 * Checks if an object exists in this collection
	 * @param	DomainObject	The object to check for
	 * @return	bool
	 */
	function exists(DomainObject $node) {
		return $this->offsetExists($node->nodeId);
	}
	
	/**
	 * Clear internal buffers
	 *
	 * Call this method after the buffered append and remove 
	 * operations have been committed to persistent storage.
	 */
	function flush() {
		$this->_append = null;
		$this->_remove = null;
	}
	
	function offsetExists($key) {
		if (isset($this->_append['node.'.$key]))
			return true;
		if (isset($this->_remove['node.'.$key]))
			return false;
		return null;
	}
	
	function offsetGet($key) {
		if (isset($this->_append['node.'.$key]))
			return $this->_append['node.'.$key];
		return null;
	}
	
	function offsetSet($key, $value) {
		$this->append($value);
	}
	
	function offsetUnset($key) {
		if (isset($this->_append['node.'.$key]))
			unset($this->_append['node.'.$key]);
	}
}

/**
 * @access	protected
 */
class BufferedDomainObjectIterator implements Iterator {
	private $_next = null;
	
	private $_append = null;
	private $_remove = null;
	private $_aa = null;
	private $_ai = 0;

	private $_offset = 0;
	private $_limit = null;
	private $_order = null;
	private $_current = null;
	private $_index = 0;
	
	function __construct(Iterator $next, $append, $remove, $offset = 0, $limit = null, $order = null) {
		$this->_next = $next;
		$this->_append = $append;
		$this->_remove = array_keys((array)$remove);
		$this->_offset = $offset;
		$this->_limit = $limit;
		$this->_order = $order;
	}
	
	function current() {
		return $this->_current;
	}
	
	function key() {
		return $this->_current->nodeId;
	}
	
	function next() {
		if (null !== $this->_limit && ++$this->_index >= $this->_limit) 
			$this->_current = null;
		else {
			if (isset($this->_aa[$this->_ai]) && $this->_aa[$this->_ai]->nodeId == $this->_current->nodeId)
				$this->_ai++;
			else
				$this->_next->next();
			$this->_current = $this->_fetch();
		}
	}
	
	function rewind() {
		$this->_aa = array_values((array)AbstractDomainObject::sort($this->_append, $this->_order));
		$this->_ai = 0;
		$this->_index = 0;
		$this->_next->rewind();
		
		if ($this->_offset) {
			for ($offset = max($this->_offset-count($this->_aa), 0); $offset < $this->_offset && $this->_next->valid() && !empty($this->_aa); $offset++) {
				if ($this->_next->current()->compare(reset($this->_aa), $this->_order) < 0)
					$this->_next->next();
				else
					array_shift($this->_aa);
			}
		}
		
		$this->_current = $this->_fetch();
	}
	
	function valid() {
		return null != $this->_current;
	}

	private function _fetch() {
		while (null != ($node = $this->_current()) && in_array($node->nodeId, $this->_remove))
			$this->_next->next();
		return $node;
	}
	
	private function _current() {
		if (isset($this->_aa[$this->_ai]) && $this->_next->valid())
			return $this->_next->current()->compare($this->_aa[$this->_ai], $this->_order) < 0 ? $this->_next->current() : $this->_aa[$this->_ai];
		if (isset($this->_aa[$this->_ai]))
			return $this->_aa[$this->_ai];
		if ($this->_next->valid())
			return $this->_next->current();
		return null;
	}
}

/**
 * Read ahead caching iterator
 * @access		public
 * @package		synd.core.model
 */
abstract class CachingDomainObjectIterator implements Iterator {
	private $_buffer = null;
	private $_keys = null;
	private $_index = 0;
	private $_count = 50;
	
	function current() {
		return $this->_buffer[$this->_keys[$this->_index]];
	}
	
	function key() {
		return $this->_keys[$this->_index];
	}
	
	function next() {
		if (++$this->_index >= $this->_count) {
			$this->_buffer = null;
			$this->_keys = null;
			$this->_index = 0;
		}
	}
	
	function rewind() {
		$this->_buffer = null;
		$this->_keys = null;
		$this->_index = 0;
	}
	
	function valid() {
		if (null === $this->_keys) {
			$this->_buffer = $this->getInstances($this->_count);
			$this->_keys = array_keys($this->_buffer);
			$this->_index = 0;
		}
		
		return isset($this->_keys[$this->_index]);
	}
	
	/**
	 * Fetch a number of instances
	 * @param	int	The number of instances to fetch
	 */
	abstract protected function getInstances($count);
}

/**
 * @access	protected
 */
class DomainObjectSortHelper {
	private $_order = null;
	
	function __construct($order) {
		$this->_order = $order;
	}
	
	function _callback_compare(DomainObject $a, DomainObject $b) {
		return $a->compare($b, $this->_order);
	}
}

/**
 * Concatenates a number of collections
 * @access		public
 * @package		synd.core.model
 */
class CompositeDomainObjectCollection implements DomainObjectCollection {
	protected $_components = null;
	
	function __construct($components) {
		$this->_components = array_values($components);
	}
	
	function getIterator($offset = 0, $limit = null, $order = null) {
		$components = $this->_components;
		for ($c=count(reset($components)); $offset-$c > 0 && !empty($components); $offset-=$c, $c=count(reset($components)))
			array_shift($components);
		return new CompositeDomainObjectIterator($components, $offset, $limit, $order);
	}
	
	function count() {
		$count = 0;
		foreach ($this->_components as $component)
			$count += count($component);
		return $count;
	}
}

/**
 * @access	protected
 */
class CompositeDomainObjectIterator implements Iterator {
	protected $_components = null;
	protected $_offset = null;
	protected $_limit = null;
	protected $_order = null;
	
	protected $_current = null;
	protected $_count = 0;
	protected $_i = 0;
	
	function __construct($components, $offset, $limit, $order) {
		$this->_components = array_values($components);
	}
	
	function current() {
		return $this->_current->current();
	}
	
	function key() {
		return $this->_current->key();
	}
	
	function next() {
		$this->_current->next();
		$this->_count++;
		
		while (!$this->_current->valid() && (null === $this->_limit || $this->_count < $this->_limit) && isset($this->_components[++$this->_i])) {
			$this->_current = $this->_components[$this->_i]->getIterator(
				max($this->_offset - $this->_count, 0), 
				null !== $this->_limit ? $this->_limit - $this->_count : $this->_limit, 
				$this->_order);
			$this->_current->rewind();
		}
	}
	
	function rewind() {
		if (!empty($this->_components)) {
			$this->_current = $this->_components[0]->getIterator($this->_offset, $this->_limit, $this->_order);
			$this->_current->rewind();
			$this->_count = 0;
			$this->_i = 0;
		}
	}
	
	function valid() {
		return null != $this->_current && $this->_current->valid();
	}
}
