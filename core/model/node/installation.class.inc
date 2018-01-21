<?php
require_once 'core/model/DatabaseEntity.class.inc';

/**
 * An installation or line item is a group of items purchased together
 *
 * Multiple items are gruped together based on purchase to track
 * invoices and other issues. The asset is used in economic systems.
 */
class synd_node_installation extends AbstractDatabaseEntity {
	function __construct($storage, $database, $data) {
		if (null != $data['DATA_FILES'])
			$data['DATA_FILES'] = unserialize($database->unwrapBinary($data['DATA_FILES']));
		parent::__construct($storage, $database, $data);
	}
	
	function __factory() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_installation');
		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_CREATE'] = time();
		parent::__factory();
	}

	function __clone() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_installation');
		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_CREATE'] = time();
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_inv_installation');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function getHandler() {
		return 'inventory';
	}

	function toString() {
		if (null != $this->data['INFO_NUMBER'])
			return $this->data['INFO_NUMBER'];
		return parent::toString();
	}

	function getInstallationNumber() {
		return $this->data['INFO_NUMBER'];
	}
	
	function setInstallationNumber($number) {
		$this->data['INFO_NUMBER'] = $number;
		$this->_save();
	}
	
	/**
	 * Permission is granted if the permission is valid for any of the items in this installation
	 */
	function hasPermission($user, $perm, $arg = null) {
		foreach (array_keys($instances = $this->getItems()) as $key) {
			if ($instances[$key]->hasPermission($user, $perm, $arg))
				return true;
		}
		if (null != ($creator = SyndNodeLib::getInstance($this->data['CREATE_NODE_ID'])) && $creator->hasAuthorization($user, $this) ||
			null != ($updator = SyndNodeLib::getInstance($this->data['UPDATE_NODE_ID'])) && $updator->hasAuthorization($user, $this))
			return true;
		if ('merge' == $perm && in_array($arg, array('INFO_NUMBER')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}
	
	function getItems() {
		$sql = "
			SELECT i.node_id FROM synd_inv_item i 
			WHERE i.installation_node_id = ".$this->_db->quote($this->nodeId);
		return SyndNodeLib::getInstances($this->_db->getCol($sql));
	}
	
	function getFiles() {
		return (array)$this->data['DATA_FILES'];
	}
	
	/**
	 * Attach a file
	 * @param	synd_type_file
	 */
	function attach($file) {
		$this->data['DATA_FILES'][] = $file;
		$this->_save();
	}

	function getIndexDocument($index) {
		return $index->createFragment($this->data['INFO_NUMBER'], 'id');
	}
	
	function _view_fileUpload(Request $request, Template $page) {
		require_once 'core/lib/SyndHTML.class.inc';
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!SyndHTML::validateFile($request, $errors, 'file'))
			return null;
		
		if (null !== ($file = SyndType::factory('file', $request['file']['tmp_name'], $request['file']['name'])))
			$this->attach($file);
	}

	function _view_fileDelete(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!isset($request['file'], $this->data['DATA_FILES'][$request['file']]))
			return null;

		if (!isset($request['confirm'])) 
			return array('content' => $page->fetchnode($this, 'confirm_delete_file.tpl',
				array('file' => $this->data['DATA_FILES'][$request['file']])));
		else {
			if (is_object($this->data['DATA_FILES'][$request['file']]))
				$this->data['DATA_FILES'][$request['file']]->delete();
			unset($this->data['DATA_FILES'][$request['file']]);
			$this->_save();
		}
	}
	
	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();
		$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_UPDATE'] = time();
	}
}
