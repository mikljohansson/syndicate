<?php
require_once 'core/model/storage/DomainStorage.class.inc';

/**
 * Static storage for special case instances
 * @package		synd.core.model
 */
class synd_storage_static extends PersistentStorage {
	function getNamespace() {
		return 'static';
	}

	function _getInstances($storage, $ids) {
		$result = array();
		foreach ($ids as $id) {
			if (null != ($class = SyndNodeLib::loadClass(substr($id, 0, strpos($id,'.')))))
				$result[$id] = new $class($storage, $id);
		}
		return $result;
	}
	
	function _factory($storage, $clsid) {
		if (null == ($class = SyndNodeLib::loadClass(substr($id, 0, strpos($id,'.')))))
			return null;
		return new $class($storage, null);
	}
	
	function rewrite($node) {}
	function delete($node) {}
	function flush() {return true;}
}
