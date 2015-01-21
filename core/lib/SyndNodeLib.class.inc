<?php
require_once 'core/lib/SyndLib.class.inc';
SyndLib::attachHook('instance', array('SyndNodeLib', '_callback_instance'), 100);
SyndLib::attachHook('instances', array('SyndNodeLib', '_callback_instances'), 100);

/**
 * Node utility class
 * @static
 * @access		public
 * @package		synd.core.lib
 */
class SyndNodeLib {
	private static $_storages = array();

	/**
	 * Returns a node fetched from its default storage
	 * @param	string		Node id
	 * @return	DomainObject
	 */
	static function getInstance($id) {
		if (null != $id)
			return self::getDefaultStorage(substr($id, 0, strpos($id,'.')))->getInstance($id);
		return null;
	}

	/**
	 * Returns a list of nodes fetched from their default storages
	 * @param	array	List of node ids
	 * @return	array
	 */
	static function getInstances($ids) {
		if (empty($ids))
			return array();
		
		$storages = array();
		foreach ($ids as $id) {
			if (null != ($clsid = substr($id, 0, strpos($id,'.')))) {
				if (!isset($storages[$clsid]))
					$storages[$clsid] = SyndNodeLib::getDefaultStorage($clsid);
				$storages[$clsid]->preload($id);
			}
		}
		
		$nodes = array();
		foreach ($ids as $id) {
			if (null != ($clsid = substr($id, 0, strpos($id,'.'))) && null != ($node = $storages[$clsid]->getInstance($id)))
				$nodes[$id] = $node;
		}
		
		return $nodes;
	}
	
	/**
	 * Preloads a node with the class default storage
	 * @param	string	Node id
	 * @return	DomainObject
	 */
	static function preload($id) {
		if (null != $id && null !== ($storage = SyndNodeLib::getDefaultStorage(substr($id, 0, strpos($id,'.')))))
			$storage->preload($id);
	}

	/**
	 * Creates a node of the specified class
	 * @param	string	clsid of node to create
	 * @return	DomainObject
	 */
	static function factory($clsid) {
		if (null !== ($storage = SyndNodeLib::getDefaultStorage($clsid)))
			return $storage->factory($clsid);
		return null;
	}

	/**
	 * Returns the default storage for a class
	 * @param	string	Class id to get storage for
	 * @return	DomainStorage
	 */
	static function getDefaultStorage($clsid) {
		if (!isset(self::$_storages[$clsid])) {
			if (null == (self::$_storages[$clsid] = call_user_func(array("synd_node_$clsid",'getDefaultStorage'))))
				self::$_storages[$clsid] = synd_storage_null::null();
		}
		return self::$_storages[$clsid];
	}
	
	/**
	 * Loads the class definition for a clsid
	 * @static
	 * @param	string	clsid to load
	 * @return	string
	 */
	static function loadClass($clsid) {
		return "synd_node_$clsid";
	}

	/**
	 * Returns the inherited clsids like 'issue.repair.replace'
	 * @param	string	The clsid to lookup
	 * @return	string
	 */
	static function getInheritedBranch($clsid) {
		if (null == ($class = SyndNodeLib::loadClass($clsid)))
			return null;
		$tree = $clsid;
		while (null != ($class = strtolower(get_parent_class($class))) && 0 === strpos($class, 'synd_node_'))
			$tree = substr($class, 10).'.'.$tree;
		return $tree;
	}

	/**
	 * @access	private
	 */
	static function _callback_instance(&$result, $id) {
		if (0 === strpos($id,'node.') && null !== ($result = SyndNodeLib::getInstance(substr($id,5))))
			return true;
	}

	/**
	 * @access	private
	 */
	static function _callback_instances(&$result, $ids) {
		$storages = array();
		foreach ($ids as $id) {
			if (0 === strpos($id, 'node.')) {
				$clsid = substr($id, 5, strpos($id,'.',5)-5);
				if (null != ($storages[$clsid] = SyndNodeLib::getDefaultStorage($clsid)))
					$storages[$clsid]->preload(substr($id,5));
			}
		}
		
		if (!empty($storages)) {
			foreach ($ids as $id) {
				if (0 === strpos($id, 'node.')) {
					$clsid = substr($id, 5, strpos($id,'.',5)-5);
					if (isset($storages[$clsid]) && null != ($node = $storages[$clsid]->getInstance(substr($id,5))))
						$result[$id] = $node;
				}
			}
		}

		return count($result) == count($ids);
	}
}
