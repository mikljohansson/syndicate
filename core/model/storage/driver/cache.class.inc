<?php
require_once 'core/model/storage/DomainStorage.class.inc';
require_once 'core/lib/CacheStrategy.class.inc';

/**
 * Caching storage
 * @package		synd.core.model
 */
class synd_storage_cache extends ForwardingStorage {
	/**
	 * @access	protected
	 */
	var $_strategy = null;
	
	/**
	 * @access	protected
	 */
	var $_namespace = null;

	/**
	 * @access	protected
	 */
	var $_ttl = 93600;

	/**
	 * @access	private
	 */
	var $_exitHandler = false;
	
	/**
	 * Ids scheduled for preload
	 * @access	private
	 */
	var $_preload = null;
	
	/**
	 * @see		CacheStrategy
	 * @param	DomainStorage	Next in chain-of-command
	 * @param	CacheStrategy	Cache implementation to use
	 */
	function __construct($next, $dsn) {
		parent::__construct($next, $dsn);
		$this->_strategy = CacheStrategyManager::factory();
		$this->_namespace = $this->getNamespace();
	}
	
	function _getInstances($storage, $ids) {
		profiler_enter(__METHOD__);
		$result = array();
		
		if (!empty($ids)) {
			$preload = $this->_preload;
			$this->_preload = null;

			$queue = array();
			foreach ($ids as $key => $id)
				$queue[$id] = $id.'.'.$this->_namespace;

			if (false != ($values = $this->_strategy->get(empty($preload) ? $queue : array_merge($queue, $preload)))) {
				foreach ($values as $cacheid => $node) {
					if (isset($queue[$node->nodeId]))
						unset($queue[$node->nodeId]);
					$result[$node->nodeId] = $node;
				}
			}

			if (!empty($queue)) {
				foreach ($this->_next->_getInstances($storage, array_keys($queue)) as $id => $node) {
					$this->_strategy->add($id.'.'.$this->_namespace, $node, $this->_ttl);
					$result[$id] = $node;
				}
			}
		}

		profiler_leave(__METHOD__);
		return $result;
	}
	
	function preload($id) {
		if (null != $id)
			$this->_preload[$id] = $id.'.'.$this->_namespace;
		$this->_next->preload($id);
	}

	function rewrite($node) {
		$this->_next->rewrite($node);
		$this->_strategy->set($node->nodeId.'.'.$this->_namespace, $node, $this->_ttl);
	}
	
	/**
	 * @deprecated
	 */
	function refresh($node) {
		$this->_next->refresh($node);
		$this->_strategy->set($node->nodeId.'.'.$this->_namespace, $node, $this->_ttl);
	}
	
	function delete($node) {
		$this->_next->delete($node);
		$this->_strategy->delete($node->nodeId.'.'.$this->_namespace);
	}	
}
