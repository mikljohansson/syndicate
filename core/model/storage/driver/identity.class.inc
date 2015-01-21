<?php
require_once 'core/model/storage/DomainStorage.class.inc';

/**
 * Identity map storage (keeps references intact)
 * @package		synd.core.model
 */
class synd_storage_identity extends ForwardingStorage {
	/**
	 * @access	protected
	 */
	var $_instances = null;
	
	function getInstance($id) {
		if (null != $id && isset($this->_instances[$id]))
			return $this->_instances[$id];
		return parent::getInstance($id);
	}

	function _getInstances($storage, $ids) {
		foreach ($ids as $key => $id) {
			if (isset($this->_instances[$id]))
				unset($ids[$key]);
		}
		
		if (!empty($ids)) {
			foreach ($this->_next->_getInstances($storage, $ids) as $id => $instance)
				$this->_instances[$id] = $instance;
		}
		
		return $this->_instances;
	}
	
	/**
	 * Blocks requests to preload existing nodes
	 */
	function preload($id) {
		if (!isset($this->_instances[$id]))
			$this->_next->preload($id);
	}

	function _factory($storage, $clsid) {
		if (null !== ($node = $this->_next->_factory($storage, $clsid))) {
			assert('null != $node->nodeId');
			$this->_instances[$node->nodeId] = $node;
		}
		return $node;
	}
	
	function copy($node) {
		if (null !== ($clone = $this->_next->copy($node))) {
			assert('null != $clone->nodeId');
			$this->_instances[$clone->nodeId] = $clone;
		}
		return $clone;
	}

	function delete($node) {
		if (isset($this->_instances[$node->nodeId]))
			unset($this->_instances[$node->nodeId]);
		$this->_next->delete($node);
	}	
}
