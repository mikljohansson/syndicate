<?php
require_once 'core/model/DatabaseEntity.class.inc';

class synd_node_os extends AbstractDatabaseEntity {
	var $_software = null;
	
	function __factory() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_os');
		$this->data['TS_CREATE'] = $this->data['TS_UPDATE'] = time();
		parent::__factory();
	}
	
	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_os');
		$this->data['TS_CREATE'] = time();
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_inv_os');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}
	
	function getParent() {
		if (null === ($parent = $this->_storage->getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function setParent($parent) {
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
		$this->_save();
	}
	
	function toString() {
		if (!empty($this->data['INFO_IDENTIFIER']))
			return $this->data['INFO_IDENTIFIER'];
		if (!empty($this->data['INFO_MACHINE_NAME']))
			return $this->data['INFO_MACHINE_NAME'];
		return parent::toString();
	}

	static function findIdentifier($parent, $identifier) {
		$sql = "
			SELECT o.node_id FROM synd_inv_os o
			WHERE 
				o.parent_node_id = '$parent->nodeId' AND
				o.info_identifier = ".$parent->_db->quote($identifier);
		if (null != ($node = $parent->_storage->getInstance($parent->_db->getOne($sql))))
			return $node;
			
		$node = $parent->appendChild(SyndNodeLib::factory('os'));
		$node->data['INFO_IDENTIFIER'] = $identifier;
		$node->save();
		return $node;
	}
	
	function getIndexDocument($index) {
		return $index->createComposite(array(
			$index->createFragment($this->data['INFO_IDENTIFIER']),
			$index->createFragment($this->data['INFO_RELEASE']),
			$index->createFragment($this->data['INFO_VERSION']),
			$index->createFragment($this->data['INFO_MACHINE_NAME']),
			$index->createFragment($this->data['INFO_LOADED_IMAGE']),
			$index->createFragment(implode(' ', SyndLib::array_collect($this->getSoftware(),'INFO_PRODUCT'))),
			));
	}
	
	function hasPermission($user, $perm, $arg = null) {
		if ('invoke' == $perm && in_array($arg, array(
			'setversion', 'setsoftware','setloadedimage','setmachinename')))
			return true;
		return parent::hasPermission($user, $perm, $arg);
	}

	function _public_begin() {
	}
	
	function _public_commit() {
		if ($this->isNew())
			$this->save();
	}
	
	function setMachineName($name) {
		if ($name != $this->data['INFO_MACHINE_NAME']) {
			$this->data['INFO_MACHINE_NAME'] = $name;
			$this->_save();
		}
	}
	
	function setVersion($release, $version) {
		if ($release != $this->data['INFO_RELEASE']) {
			$this->data['INFO_RELEASE'] = $release;
			$this->_save();
		}
		if ($version != $this->data['INFO_VERSION']) {
			$this->data['INFO_VERSION'] = $version;
			$this->_save();
		}
	}
	
	function setLoadedImage($name) {
		if ($name != $this->data['INFO_LOADED_IMAGE']) {
			$this->data['INFO_LOADED_IMAGE'] = $name;
			$this->_save();
		}
	}

	function setSoftware($software) {
		if (empty($software) || $this->data['INFO_SOFTWARE_HASH'] == ($hash = SyndLib::hash($software)))
			return;
		$this->data['INFO_SOFTWARE_HASH'] = $hash;
		$this->_software = $software;
		$this->_save();
	}
	
	function getSoftware() {
		$sql = "
			SELECT s.* FROM synd_inv_software s
			WHERE s.os_node_id = '$this->nodeId'
			ORDER BY s.info_product, s.info_version";
		return $this->_db->getAll($sql);
	}

	function _entity_onpreflush() {
		parent::_entity_onpreflush();
		$this->data['TS_UPDATE'] = time();
	}
	
	function _entity_onpostflush() {
		parent::_entity_onpostflush();
		
		if (null !== $this->_software) {
			$sql = "
				DELETE FROM synd_inv_software
				WHERE os_node_id = '$this->nodeId'";
			$this->_db->query($sql);

			$rows = array();
			foreach ($this->_software as $product) {
				if (empty($product[0]) || empty($product[1]))
					continue;
				$rows[SyndLib::hash($this->nodeId, $product[0], $product[1])] = array(
					'OS_NODE_ID'	=> $this->nodeId,
					'INFO_PRODUCT' 	=> $product[0],
					'INFO_VERSION'	=> $product[1],
					'INFO_STATE'	=> $product[2]);
			}

			$this->_db->load('synd_inv_software', $rows);
			
			$this->_software = null;
			$this->_node_onchange_internal();
		}
	}
}
