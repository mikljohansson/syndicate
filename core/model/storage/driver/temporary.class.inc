<?php
require_once 'core/model/storage/DomainStorage.class.inc';
require_once 'core/lib/CacheStrategy.class.inc';

/**
 * Temporary garbage collected storage
 * @package		synd.core.model
 */
class synd_storage_temporary extends ForwardingStorage {
	/**
	 * @access	protected
	 */
	var $_db = null;
	
	/**
	 * @access	protected
	 */
	var $_strategy = null;
	
	/**
	 * @access	protected
	 */
	var $_ttl = 93600;
	
	/**
	 * @access	protected
	 */
	var $_namespace = null;
	
	/**
	 * @access	protected
	 */
	var $_dirty = null;
	
	/**
	 * @access	protected
	 */
	var $_deleted = null;
	
	/**
	 * @access	protected
	 */
	var $_exitHandler = false;
	
	/**
	 * @param	DomainStorage	Next in chain-of-command
	 * @param	Database		Database containing the 'synd_variable' table
	 * @param	integer				Seconds to live for nodes (default 26 hours)
	 */
	function __construct($next, $dsn) {
		global $synd_maindb;
		parent::__construct($next, $dsn);
		$this->_db = $synd_maindb;
		$this->_namespace = 'heap.'.md5($this->_next->getNamespace().'.'.session_id());
		$this->_strategy = CacheStrategyManager::factory();
	}

	function _getInstances($storage, $ids) {
		if (!isset($_SESSION[$this->_namespace]) || !count($temporary = array_intersect($ids, $_SESSION[$this->_namespace]))) 
			return $this->_next->_getInstances($storage, $ids);

		// Load temporary instances
		$result = array();
		foreach ((array)$this->_strategy->get(SyndLib::array_prepend($temporary, $this->_namespace.'.')) as $value) {
			if (false !== ($node = unserialize($value))) {
				unset($ids[array_search($node->nodeId, $ids)]);
				$node->__tempchecksum = md5($value);
				$this->_store($node);
				$result[$node->nodeId] = $node;
			}
		}
		$temporary = array_diff($temporary, array_keys($result));
		
		for ($i=0, $cnt=count($temporary); $i<$cnt; $i+=1000) {
			$sql = "
				SELECT v.value FROM synd_variable v
				WHERE 
					v.namespace = ".$this->_db->quote($this->_namespace)." AND
					v.variable IN (".implode(',',$this->_db->quote(array_slice($temporary,$i,1000))).")";

			foreach ($this->_db->getCol($sql) as $value) {
				// Need to unwrap to ensure checksum is calculated correctly
				$value = $this->_db->unwrapBinary($value);

				if (false !== ($node = unserialize($value))) {
					unset($ids[array_search($node->nodeId, $ids)]);
					$node->__tempchecksum = md5($value);
					$this->_store($node);
					$result[$node->nodeId] = $node;
				}
			}
		}
		
		// Remove artefacts from abandoned and garbage collected edits
		if (count($temporary) > count($result)) {
			foreach (array_diff($temporary, array_keys($result)) as $id)
				unset($_SESSION[$this->_namespace][array_search($id, $_SESSION[$this->_namespace])]);
		}

		// Load instances from next storage if non-temporary ids requested
		if (!empty($ids)) {
			foreach (array_keys($instances = $this->_next->_getInstances($storage, $ids)) as $key)
				$result[$key] = $instances[$key];
		}
		
		return $result;
	}
	
	/**
	 * Stores the node on heap until rewrite()
	 */
	function _factory($storage, $clsid) {
		if (null !== ($node = $this->_next->_factory($storage, $clsid))) {
			$_SESSION[$this->_namespace][] = $node->nodeId;
			$this->_store($node);
		}
		return $node;
	}
	
	/**
	 * Stores the node on heap until rewrite()
	 */
	function copy($node) {
		if (null !== ($clone = $this->_next->copy($node))) {
			$_SESSION[$this->_namespace][] = $clone->nodeId;
			$this->_store($clone);
		}
		return $clone;
	}

	/**
	 * Removes the node from heap
	 */
	function rewrite($node) {
		if (isset($_SESSION[$this->_namespace]) && in_array($node->nodeId, $_SESSION[$this->_namespace]))
			$this->_remove($node);
		$this->_next->rewrite($node);
	}
	
	/**
	 * Refreshes and blocks chain for nodes stored on heap
	 */
	function refresh($node) {
		if (isset($_SESSION[$this->_namespace]) && in_array($node->nodeId, $_SESSION[$this->_namespace]))
			$this->_store($node);
		else
			$this->_next->refresh($node);
	}
	
	/**
	 * Stores the node on heap until rewrite()
	 */
	function checkout($node) {
		if (!isset($_SESSION[$this->_namespace]) || !in_array($node->nodeId, $_SESSION[$this->_namespace])) {
			$_SESSION[$this->_namespace][] = $node->nodeId;
			$this->_store($node);
		}
		$this->_next->checkout($node);
	}

	function delete($node) {
		if (isset($_SESSION[$this->_namespace]) && in_array($node->nodeId, $_SESSION[$this->_namespace]))
			$this->_remove($node);
		$this->_next->delete($node);
	}

	function flush() {
		return $this->_next->flush() && $this->_flush();
	}
	
	/**
	 * @access	private
	 * @return	bool	Returns true on success, false on failure
	 */
	function _flush() {
		profiler_enter(__METHOD__);
	
		if (!empty($this->_dirty)) {
			foreach ($this->_dirty as $id => $node) {
				if (isset($node->__tempchecksum)) {
					$checksum = $node->__tempchecksum;
					unset($node->__tempchecksum);
				}
				else {
					$checksum = null;
				}
				
				$value = serialize($node);
				
				if (null === $checksum || md5($value) != $checksum) {
					$data = array(
						'NAMESPACE'	=> $this->_namespace,
						'VARIABLE'	=> $id,
						'EXPIRES'	=> time() + $this->_ttl,
						'VALUE'		=> $this->_db->wrapBinary($value));
					$this->_db->replace('synd_variable', $data);
					$this->_strategy->set($this->_namespace.'.'.$id, $value, $this->_ttl);
				}
			}
			
			$this->_dirty = null;
		}
		
		// Garbage collect outdated nodes
		$sql = "
			SELECT v.variable, v.value FROM synd_variable v
			WHERE 
				v.namespace = ".$this->_db->quote($this->_namespace)." AND 
				v.expires < ".time();
		foreach ($this->_db->getAssoc($sql) as $id => $value) {
			if ((!is_array($this->_deleted) || !in_array($id, $this->_deleted)) && false !== ($node = unserialize($this->_db->unwrapBinary($value)))) {
				if (null === $this->_next->getInstance($node->nodeId))
					$node->delete();
				unset($node);
			}
			$this->_deleted[] = $id;
			$this->_strategy->delete($this->_namespace.'.'.$id);
		}

		// Batch delete entries
		if (!empty($this->_deleted)) {
			for ($i=0, $cnt=count($this->_deleted); $i<$cnt; $i+=1000) {
				$sql = "
					DELETE FROM synd_variable
					WHERE 
						namespace = ".$this->_db->quote($this->_namespace)." AND 
						variable IN (".implode(',',$this->_db->quote(array_slice($this->_deleted,$i,1000))).")";
				$this->_db->query($sql);
			}
			
			$this->_deleted = null;
		}

		profiler_leave(__METHOD__);
		return true;
	}
	
	/**
	 * @access	private
	 */
	function _callback_flush() {
		$this->_flush();
	}

	/**
	 * @access	private
	 */
	function _store($node) {
		if (!$this->_exitHandler) {
			$this->_exitHandler = true;
			SyndLib::attachHook('shutdown', array($this, '_callback_flush'), -200);
		}
		
		$this->_dirty[$node->nodeId] = $node;
	}
	
	/**
	 * @access	private
	 */
	function _remove($node) {
		if (!$this->_exitHandler) {
			$this->_exitHandler = true;
			SyndLib::attachHook('shutdown', array($this, '_callback_flush'), -200);
		}
	
		unset($_SESSION[$this->_namespace][array_search($node->nodeId, $_SESSION[$this->_namespace])]);
		if (isset($this->_dirty[$node->nodeId]))
			unset($this->_dirty[$node->nodeId]);

		$this->_deleted[] = $node->nodeId;
	}
}
