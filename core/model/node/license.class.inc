<?php
require_once 'core/model/DatabaseEntity.class.inc';

/**
 * Software license
 *
 * Software found by the inventory agent can be matched against 
 * these licenses to produce license usage reports for a site.
 */
class synd_node_license extends AbstractDatabaseEntity {
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
		return self::createQuery($storage, 'synd_inv_license');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('merge' == $perm && in_array($arg, array(
			'PARENT_NODE_ID','INFO_MAKE','INFO_PRODUCT','INFO_DESC',
			'INFO_COST','INFO_LICENSES','FLAG_SITE_LICENSE')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}

	function getHandler() {
		return 'inventory';
	}
	
	function toString() {
		if (null != $this->data['INFO_PRODUCT'])
			return $this->data['INFO_PRODUCT'];
		if ($this->isNew())
			return SyndLib::translate('New software license');
		return parent::toString();
	}

	function setParent($parent) {
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
		$this->_save();
	}
	
	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	/**
	 * Returns the number of licenses bought
	 * @return	integer
	 */
	function getLicenses() {
		return $this->data['INFO_LICENSES'];
	}
	
	/**
	 * Returns the number of licenses in use
	 * @return	integer
	 */
	function getUsedLicenses() {
		
	}
	
	/**
	 * Returns the folders for which this license should be used
	 * @return	array
	 */
	function getFolders() {
		$sql = "
			SELECT lf.folder_node_id FROM synd_inv_license_folder lf
			WHERE lf.license_node_id = ".$this->_db->quote($this->nodeId);
		return SyndNodeLib::getInstances($this->_db->getCol($sql));
	}
	
	function _view_addFolder(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (null === ($folder = SyndNodeLib::getInstance($request['folder'])))
			throw new InvalidArgumentException();
		if (in_array($folder->nodeId, SyndLib::collect($this->getFolders(),'nodeId')))
			return null;
		
		$sql = "
			INSERT INTO synd_inv_license_folder
			(license_node_id, folder_node_id)
			VALUES
			(".$this->_db->quote($this->nodeId).",
			 ".$this->_db->quote($folder->nodeId).")";
		$this->_db->query($sql);
	}
	
	function _view_removeFolder(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (null === ($folder = SyndNodeLib::getInstance($request[2])))
			throw new InvalidArgumentException();
		
		$sql = "
			DELETE FROM synd_inv_license_folder
			WHERE 
				license_node_id = ".$this->_db->quote($this->nodeId)." AND
				folder_node_id = ".$this->_db->quote($folder->nodeId);
		$this->_db->query($sql);
	}
	
	/**
	 * Returns the software filters
	 * @return	array
	 */
	function getSoftwareFilters() {
		$sql = "
			SELECT ls.info_filter FROM synd_inv_license_software ls
			WHERE ls.license_node_id = ".$this->_db->quote($this->nodeId);
		return $this->_db->getCol($sql);
	}
	
	function _view_addSoftwareFilter(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!isset($request['filter']))
			throw new InvalidArgumentException();
		if (in_array($request['filter'], $this->getSoftwareFilters()))
			return null;
		
		$sql = "
			INSERT INTO synd_inv_license_software
			(license_node_id, info_filter)
			VALUES
			(".$this->_db->quote($this->nodeId).",
			 ".$this->_db->quote($request['filter']).")";
		$this->_db->query($sql);
	}

	function _view_delSoftwareFilter(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!isset($request[2]))
			throw new InvalidArgumentException();
		
		$sql = "
			DELETE FROM synd_inv_license_software
			WHERE 
				license_node_id = ".$this->_db->quote($this->nodeId)." AND
				info_filter = ".$this->_db->quote($request[2]);
		$this->_db->query($sql);
	}

	/**
	 * Is this a site (unlimited) license
	 * @return	bool
	 */
	function isSiteLicense() {
		return $this->data['FLAG_SITE_LICENSE'];
	}

	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();
		$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_UPDATE'] = time();
	}
}
