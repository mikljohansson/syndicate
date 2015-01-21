<?php
require_once 'core/model/DatabaseEntity.class.inc';
require_once 'core/model/node/lease.class.inc';

/**
 * Inventory relation between an item and a client or lease
 */
class synd_node_used extends AbstractDatabaseEntity {
	function __construct($storage, $database, $data) {
		parent::__construct($storage, $database, $data);
		SyndNodeLib::preload($this->data['PARENT_NODE_ID']);
		SyndNodeLib::preload($this->data['CHILD_NODE_ID']);
	}
	
	function __factory() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_used');
		$this->data['TS_CREATE'] = $this->data['TS_UPDATE'] = time();
		$this->data['CREATE_NODE_ID'] = $this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		parent::__factory();
	}

	function __clone() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_inv_used');
		$this->data['TS_CREATE'] = time();
		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_inv_used');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}
	
	function toString() {
		$child = $this->getChild();
		return $child->toString(); 
	}

	function isInheritedFrom($class) {
		if ('synd_node_item' == $class)
			return true;
		return parent::isInheritedFrom($class);
	}
	
	function hasPermission($user, $perm, $arg = null) {
		$parent = $this->getParent();
		if ($parent->isPermitted($perm, $arg))
			return true;
		if ('write' == $perm && $parent->hasAuthorization($user, $this))
			return true;

		$child = $this->getChild();
		if ($child->isPermitted($perm, $arg))
			return true;
			
		return false;
	}

	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function setParent($node) {
		$this->data['PARENT_NODE_ID'] = $node->nodeId;
		$this->_save();
	}

	function getChild() {
		if (null === ($child = SyndNodeLib::getInstance($this->data['CHILD_NODE_ID'])))
			$child = SyndNodeLib::getInstance('null.null');
		return $child;
	}
	
	function setChild($node) {
		$this->data['CHILD_NODE_ID'] = $node->nodeId;
		$this->_save();
	}
	
	function getCreator() {
		if (null === ($creator = SyndNodeLib::getInstance($this->data['CREATE_NODE_ID'])))
			$creator = SyndNodeLib::getInstance('user_null.null');
		return $creator;
	}
	
	function getItem() {
		return $this->getChild();
	}
	
	function setItem($item) {
		$this->setChild($item);
	}
	
	function getCustomer() {
		$client = $this->getParent();
		if ($client->isNull())
			$client = SyndNodeLib::getInstance('user_null.null');
		return $client;
	}
	
	function getFolder() {
		$item = $this->getItem();
		return $item->getFolder();
	}

	function setCustomer($client) {
		$this->setParent($client);
	}
	
	function isActive() {
		return null == $this->data['TS_EXPIRE'];
	}
	
	function isInactive() {
		return !$this->isActive();
	}
	
	function setInactive() {
		if ($this->isActive()) {
			$lease = $this->getParent();
			$lease->removeChild($this);
			$this->data['TS_EXPIRE'] = time();
			$this->_save();
		}
	}
	
	function validate($data) {
		require_once 'core/lib/SyndDate.class.inc';
		$errors = array();
		if (empty($data['item']))
			$errors['item'] = SyndLib::translate('Select a valid item');
		if (isset($data['TS_EXPIRE']) && '' != $data['TS_EXPIRE']) {
			if (!SyndDate::checkdate($data['TS_EXPIRE'])) 
				$errors['TS_EXPIRE'] = SyndLib::translate("Invalid date given ('%s')", tpl_chop($data['TS_EXPIRE'],12));
			unset($data['TS_EXPIRE']);
		}
		return array_merge(parent::validate($data), $errors);
	}
	
	function merge($data) {
		if (!empty($data['TS_EXPIRE']))
			$data['TS_EXPIRE'] = strtotime($data['TS_EXPIRE']);
		
		if (!empty($data['item']) && null != ($item = SyndNodeLib::getInstance($data['item']))) {
			$item->_node_onchange_internal();
			$current = $this->getItem();
			$current->_node_onchange_internal();
			$inventory = Module::getInstance('inventory');

			if ($current->nodeId != $item->nodeId) {
				$tsExpire = $this->data['TS_EXPIRE'];

				$current = $this->getItem();
				$current->setCustomer(SyndNodeLib::getInstance('null.null'));
				if (null !== $inventory->getTerminateFolder())
					$current->setFolder($inventory->getTerminateFolder());

				$relation = $item->_setCustomer($this->getCustomer());
				if (null != $relation) {
					$relation->date['TS_CREATE'] = $tsExpire;
					$relation->_save();
				}

				unset($data['TS_EXPIRE']);
			}
		}
		parent::merge($data);
	}
	
	function _findItems($string) {
		$inventory = Module::getInstance('inventory');
		return $inventory->findItems($string);
	}

	/**
	 * Terminates this leasing
	 */
	function terminate() {
		if ($this->isInactive())
			return false;
		
		$item = $this->getItem();

		if (!$item->isNull()) {
			$itemClient = $item->getCustomer();
			$client = $this->getCustomer();
			if ($itemClient->nodeId == $client->nodeId)
				$item->setCustomer(SyndNodeLib::getInstance('null.null'));
		}
		
		$this->setInactive();
		$this->flush();
		return true;
	}
	
	function getExpire() {
		$client = $this->getCustomer();
		if (($client instanceof synd_node_lease))
			return $client->getExpire();
		return null;
	}

	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();
		$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_UPDATE'] = time();
	}
}
