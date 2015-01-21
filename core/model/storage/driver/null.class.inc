<?php
require_once 'core/model/storage/DomainStorage.class.inc';

/**
 * Null object pattern storage
 * @package		synd.core.model
 */
class synd_storage_null implements DomainStorage {
	protected static $_instance = null;
	
	static function null() {
		if (null == self::$_instance)
			self::$_instance = new synd_storage_null();
		return self::$_instance;
	}
	
	static function getStorage($dsn, DomainStorage $next) {
		return self::null();
	}

	function getDSN() {
		return array('null');
	}

	function getNamespace()					{return null;}
	function getInstance($id)				{return null;}
	function getInstances($ids)				{return array();}
	function _getInstances($storage, $ids)	{return array();}
	function preload($id)					{}

	function factory($clsid)				{return null;}
	function _factory($storage, $clsid)		{return null;}
	function copy($node)					{return null;}

	function rewrite($node)					{}
	function refresh($node)					{}
	function checkout($node)				{}
	function delete($node)					{}
	function flush()						{}

	function getPersistentStorage()			{return $this;}
	function getDatabase()					{return DatabaseManager::getConnection('null://null');}
}
