<?php
require_once 'core/model/DomainObjectCollection.class.inc';
require_once 'core/model/DatabaseEntity.class.inc';

/**
 * Collection of database entities
 * @access		public
 * @package		synd.core.model
 */
class DatabaseEntityCollection implements DomainObjectCollection {
	protected $_storage = null;
	protected $_query = null;
	
	function __construct(DomainStorage $storage, EntityQuery $query) {
		$this->_storage = $storage;
		$this->_query = $query;
	}
	
	function getIterator($offset = 0, $limit = null, $order = null) {
		$query = clone $this->_query;
		$query->column($query->getPrimaryKey());
		
		if (!empty($order)) {
			foreach ($order as $i => $column) {
				if (is_string($column) && !is_numeric($column) && !empty($column)) {
					$query->column($column);
					$query->order($column, !isset($order[$i+1]) || !empty($order[$i+1]));
				}
			}
		}
		
		return new DatabaseEntityIterator($this->_storage, $query, $offset, $limit);
	}
	
	function count() {
		return $this->_query->count();
	}
}

/**
 * One-to-many entity relation.
 *
 * The children have their parent set via the PARENT_NODE_ID columns
 * and are assumed to be cascade-on-delete
 */
class AggregateEntityCollection extends BufferedDomainObjectCollection {
	protected $_parent = null;
	protected $_class = null;
	protected $_column = null;

	protected $_query = null;
	protected $_cachesize = 250;
	protected $_cache = null;
	protected $_count = null;
	
	function __construct(DatabaseEntity $parent, $class, $column = 'PARENT_NODE_ID') {
		$this->_parent = $parent;
		$this->_class = $class;
		$this->_column = $column;
	}
	
	function __clone() {
		if (null != $this->_query)
			$this->_query = clone $this->_query;
	}
	
	/**
	 * Don't serialize the database query
	 */
	function __sleep() {
		//if (null === $this->_cache)
		//	$this->_cache();
		$vars = array_keys(get_object_vars($this));
		unset($vars[array_search('_query', $vars)]);
		return $vars;
	}

	private function _cache() {
		profiler_enter(__METHOD__);

		if (0 == ($this->_count = count($query = $this->getDatabaseQuery()))) 
			$this->_cache = array();
		else {
			$query->column($query->getPrimaryKey());
			$this->_cache = (array)$query->getCol(0, 0, $this->_cachesize);
		}
		
		profiler_leave(__METHOD__);
	}
	
	function _getIterator($offset = 0, $limit = null, $order = null) {
		if (null === $this->_cache)
			$this->_cache();
		if (empty($this->_cache))
			return new EmptyIterator();
		
		if ($offset + $limit < $this->_cachesize && (null != $limit || count($this->_cache) < $this->_cachesize) && null == $order)
			return new ArrayIterator(SyndNodeLib::getInstances(array_slice($this->_cache, $offset, null != $limit ? $limit : $this->_cachesize)));
		
		$query = $this->getDatabaseQuery();
		$query->column($query->getPrimaryKey());
		
		if (!empty($order)) {
			foreach ($order as $i => $column) {
				if (is_string($column) && !is_numeric($column) && !empty($column)) {
					$query->column($column);
					$query->order($column, !isset($order[$i+1]) || !empty($order[$i+1]));
				}
			}
		}
		
		return new DatabaseEntityIterator($this->_parent->_storage, $query, $offset, $limit);
	}
	
	function _count() {
		if (null === $this->_cache)
			$this->_cache();
		return $this->_count;
	}
	
	function getDatabaseQuery() {
		if (null == $this->_query) {
			$this->_query = call_user_func(array($this->_class, 'getEntityQuery'), $this->_parent->_storage);
			$this->_query->where($this->_column, $this->_parent->nodeId);
		}
		return $this->_query;
	}
	
	function append(DomainObject $node) {
		$node->data[$this->_column] = $this->_parent->nodeId;
		$this->_parent->_node_onchange_internal();
		return parent::append($node);
	}
	
	function remove(DomainObject $node) {
		if ($node->data[$this->_column] == $this->_parent->nodeId)
			$node->data[$this->_column] = null;
		$this->_parent->_node_onchange_internal();
		return parent::remove($node);
	}

	function offsetExists($key) {
		if (null === $this->_cache)
			$this->_cache();
		if (is_bool($result = parent::offsetExists($key)))
			return $result;
		if (count($this->_cache) < $this->_cachesize) 
			return in_array($key, $this->_cache);
		if (in_array($key, $this->_cache))
			return true;
		$query = $this->getDatabaseQuery();
		$query->column($query->getPrimaryKey());
		$query->where($query->getPrimaryKey(), $key);
		return $query->getOne() == $key;
	}
	
	function flush() {
		parent::flush();

		$this->_cache = null;
		$this->_count = null;
	}
}

/**
 * Many-to-many entity relation
 *
 * A separate table is used to hold the relation between the parent
 * and child.
 */
class AssociateEntityCollection extends BufferedDomainObjectCollection {
	protected $_parent = null;
	protected $_table = null;
	protected $_class = null;
	protected $_pcol = null;
	protected $_ccol = null;

	protected $_query = null;
	protected $_cachesize = 250;
	protected $_cache = null;
	protected $_count = null;
	
	protected $_insert = null;
	protected $_delete = null;
	protected $_prefilter = null;
	
	function __construct(DatabaseEntity $parent, $table, $class = null, $pcol = 'PARENT_NODE_ID', $ccol = 'CHILD_NODE_ID', $prefilter = null) {
		assert('null == $prefilter || is_callable($prefilter)');
		parent::__construct($parent);
		$this->_table = $table;
		$this->_class = $class;
		$this->_pcol = $pcol;
		$this->_ccol = $ccol;
		$this->_prefilter = $prefilter;
	}
	
	function __clone() {
		if (null != $this->_query)
			$this->_query = clone $this->_query;
	}
	
	/**
	 * Don't serialize the database query
	 */
	function __sleep() {
		//if (null === $this->_cache)
		//	$this->_cache();
		$vars = array_keys(get_object_vars($this));
		unset($vars[array_search('_query', $vars)]);
		return $vars;
	}

	private function _cache() {
		profiler_enter(__METHOD__);
		
		if (0 == ($this->_count = count($query = $this->getDatabaseQuery()))) 
			$this->_cache = array();
		else {
			$query->column($query->getPrimaryKey());
			$this->_cache = (array)$query->getCol(0, 0, $this->_cachesize);
		}
		
		$this->_parent->_node_onchange_internal();
		
		profiler_leave(__METHOD__);
	}
	
	function _getIterator($offset = 0, $limit = null, $order = null) {
		if (null === $this->_cache)
			$this->_cache();
		if (empty($this->_cache))
			return new EmptyIterator();

		if ($offset + $limit < $this->_cachesize && (null != $limit || count($this->_cache) < $this->_cachesize) && null == $order) {
			$ids = array_slice($this->_cache, $offset, null != $limit ? $limit : $this->_cachesize);
			return new ArrayIterator(null != $this->_class ? SyndNodeLib::getInstances($ids) : SyndNodeLib::getInstances($ids));
		}
		
		$query = $this->getDatabaseQuery();
		$query->column($query->getPrimaryKey());
		
		if (!empty($order)) {
			foreach ($order as $i => $column) {
				if (is_string($column) && !is_numeric($column) && !empty($column)) {
					$query->column($column);
					$query->order($column, !isset($order[$i+1]) || !empty($order[$i+1]));
				}
			}
		}
		
		return new DatabaseEntityIterator(null != $this->_class ? $this->_parent->_storage : null, $query, $offset, $limit);
	}
	
	function _count() {
		if (null === $this->_cache)
			$this->_cache();
		return $this->_count;
	}
	
	function getDatabaseQuery() {
		if (null == $this->_query) {
			if (null != $this->_class) {
				$this->_query = call_user_func(array($this->_class, 'getEntityQuery'), $this->_parent->_storage);
				$a = $this->_query->join($this->_table, null, false);
				$this->_query->where("$a.{$this->_pcol}", $this->_parent->nodeId);
				$this->_query->where("$a.{$this->_ccol} = ".$this->_query->getPrimaryKey());
			}
			else {
				$this->_query = new DatabaseEntityQuery($this->_parent->_storage, $this->_parent->_db->createQuery(), $this->_table, $this->_ccol);
				$a = $this->_query->join($this->_table, null, false);
				$this->_query->where("$a.{$this->_pcol}", $this->_parent->nodeId);
			}
		}
		return $this->_query;
	}
	
	function append(DomainObject $node) {
		$this->_parent->_node_onchange_internal();
		return parent::append($node);
	}
	
	function remove(DomainObject $node) {
		$this->_parent->_node_onchange_internal();
		return parent::remove($node);
	}
	
	function offsetExists($key) {
		if (null === $this->_cache)
			$this->_cache();
		if (is_bool($result = parent::offsetExists($key)))
			return $result;
		if (count($this->_cache) < $this->_cachesize) 
			return in_array($key, $this->_cache);
		if (in_array($key, $this->_cache))
			return true;
		$query = $this->getDatabaseQuery();
		$a = $this->_query->join($this->_table, null, false);
		$query->column("$a.{$this->_ccol}");
		$query->where("$a.{$this->_ccol}", $key);
		return $query->getOne() == $key;
	}
	
	function flush() {
		if (!empty($this->_remove)) {
			foreach (SyndNodeLib::getInstances(array_keys((array)$this->_remove)) as $child) {
				$row = array(
					$this->_pcol => $this->_parent->nodeId,
					$this->_ccol => $child->nodeId);
				$this->_parent->_db->delete($this->_table, $row);
			}
		}
		
		if (!empty($this->_append)) {
			foreach (SyndNodeLib::getInstances(array_keys((array)$this->_append)) as $child) {
				$row = array(
					$this->_pcol => $this->_parent->nodeId,
					$this->_ccol => $child->nodeId);
				$this->_parent->_db->replace($this->_table, $this->_filter($row));
			}
		}

		parent::flush();
		
		$this->_cache = null;
		$this->_count = null;
	}
	
	function _filter($row) {
		if (null != $this->_prefilter)
			return call_user_func($this->_prefilter, $row);
		return $row;
	}
}

/**
 * Iterates over an entity query
 */
class DatabaseEntityIterator extends CachingDomainObjectIterator {
	protected $_storage = null;
	protected $_query = null;
	protected $_result = null;
	protected $_offset = 0;
	protected $_limit = null;
	
	function __construct($storage, DatabaseQuery $query, $offset = 0, $limit = null) {
		$this->_storage = $storage;
		$this->_query = $query;
		$this->_offset = $offset;
		$this->_limit = $limit;
	}
	
	function rewind() {
		parent::rewind();
		$this->_result = null;
	}
	
	protected function getInstances($count) {
		if (null === $this->_result) {
			$this->_result = $this->_query->getIterator($this->_offset, $this->_limit);
			$this->_result->rewind();
		}

		$ids = array();
		for ($i=0; $i<$count && $this->_result->valid(); $i++, $this->_result->next())
			$ids[] = $this->_query->getIdentifier($this->_result->current());
		
		return null != $this->_storage ? 
			$this->_storage->getInstances($ids) : 
			SyndNodeLib::getInstances($ids);
	}
}
