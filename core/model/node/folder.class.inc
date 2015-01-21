<?php
require_once 'core/model/node/instance.class.inc';
require_once 'core/model/Collection.class.inc';

/**
 * Inventory folder
 */
class synd_node_folder extends synd_node_instance implements Collection {
	/**
	 * Query collection for this folder
	 * @access	private
	 */
	var $_collection = null;
	
	/**
	 * Cached ids of child folderes
	 * @access	private
	 */
	var $_children = null;
	
	function __factory() {
		global $synd_user;
		$this->data['CREATE_NODE_ID'] = $this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		parent::__factory();
	}

	function __clone() {
		global $synd_user;
		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;
		parent::__clone();
	}

	function __sleep() {
		$vars = parent::__sleep();
		unset($vars[array_search('_collection', $vars)]);
		return $vars;
	}
	
	static function getEntityQuery(DomainStorage $storage) {
		$query = parent::getEntityQuery($storage);
		$query->join('synd_inv_folder');
		return $query;
	}

	function hasPermission($user, $perm, $arg = null) {
		$creator = $this->getCreator();
		if ('read' == $perm && null != $arg)
			return false;
		if ('admin' == $perm && $creator->hasAuthorization($user, $this))
			return true;
		if ('append' == $perm && $this->isValidChild($arg) && $this->isPermitted('write'))
			return true;
		if ('remove' == $perm && $this->isPermitted('write'))
			return true;
		if ('merge' == $perm && in_array($arg, array(
			'ACCEPT_CLASS_IDS','INFO_HEAD','INFO_DESC','INFO_BODY')))
			return $this->hasPermission($user, 'write');
		return parent::hasPermission($user, $perm, $arg);
	}

	/**
	 * Override since parent may be null instance
	 */
	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function getHandler() {
		return 'inventory';
	}
	
	function getIndexDocument($index) {
		return $index->createComposite(array(
			$index->createFragment($this->toString(),'title'),
			$index->createFragment($this->data['INFO_DESC'],'description'),
			$index->createFragment($this->data['INFO_BODY'],'body'),
			));
	}

	function getCreator() {
		if (null === ($creator = SyndNodeLib::getInstance($this->data['CREATE_NODE_ID'])))
			$creator = SyndNodeLib::getInstance('user_null.null');
		return $creator;
	}
	
	function toString() {
		return $this->data['INFO_HEAD'];
	}

	function setTitle($title) {
		$this->data['INFO_HEAD'] = $title;
	}
	
	function getDescription() {
		return $this->data['INFO_DESC'];
	}

	/**
	 * Checks if an object could reside in this folder
	 * @access	protected
	 */
	function isValidChild($child) {
		if (($child instanceof synd_node_folder))
			return true;
		foreach ($this->getClasses() as $clsid) {
			$class = "synd_node_$clsid";
			if ($child instanceof $class)
				return true;
		}
		return false;
	}
	
	/**
	 * Checks if an class could reside in this folder
	 * @access	protected
	 */
	function isValidClass($child) {
		foreach ($this->getClasses() as $class) {
			if ("synd_node_$class" == $child || is_subclass_of($child, "synd_node_$class"))
				return true;
		}
		return false;
	}

	function appendChild($child) {
		if (($child instanceof self)) {
			if (null === $this->_children)
				$this->getChildren();
			if (false === array_search($child->nodeId, $this->_children)) {
				$this->_children[] = $child->nodeId;
				$this->_node_onchange_internal();
			}
		}
		return parent::appendChild($child);
	}

	function removeChild($child) {
		if (($child instanceof self)) {
			if (null !== $this->_children && false !== ($key = array_search($child->nodeId, $this->_children))) {
				unset($this->_children[$key]);
				$this->_node_onchange_internal();
			}
		}
		return parent::removeChild($child);
	}

	function getChildren() {
		if (null === $this->_children) {
			$sql = "
				SELECT i.node_id FROM synd_instance i, synd_inv_folder f
				WHERE 
					i.node_id = f.node_id AND
					i.parent_node_id = ".$this->_db->quote($this->nodeId);
			$this->_children = (array)$this->_db->getCol($sql);
			$this->_node_onchange_internal();
		}
		return $this->_storage->getInstances($this->_children);
	}
	
	function getCollections() {
		$collections = array();
		foreach ($this->getClasses() as $clsid) {
			$query = call_user_func(array(SyndNodeLib::loadClass($clsid), 'getEntityQuery'), $this->_storage);
			$query->where('parent_node_id = '.$this->_db->quote($this->nodeId));
			$collections[] = SyndType::factory('query_collection', SyndNodeLib::getDefaultStorage($clsid), $query->toString());
		}
		return $collections;
	}
	
	/**
	 * @return	Collection
	 */
	function getCollection() {
		if (null === $this->_collection) {
			$fragments = array_merge($this->getCollections(), SyndLib::invoke($this->getFolders(), 'getCollection'));
			$this->_collection = SyndType::factory('composite_collection', $fragments);
		}
		return $this->_collection;
	}
	
	/**
	 * @see		Collection::getCount()
	 */
	function getCount() {
		$collection = $this->getCollection();
		return $collection->getCount();
	}
	
	/**
	 * @see		Collection::getContents()
	 */
	function getContents($offset = 0, $limit = null) {
		$collection = $this->getCollection();
		return $collection->getContents($offset, $limit);
	}
	
	/**
	 * @see		Collection::getFilteredCount()
	 */
	function getFilteredCount($classes) {
		$collection = $this->getCollection();
		return $collection->getFilteredCount($classes);
	}

	/**
	 * @see		Collection::getFilteredContents()
	 */
	function getFilteredContents($classes) {
		$collection = $this->getCollection();
		return $collection->getFilteredContents($classes);
	}

	function getFolders() {
		return $this->getChildren();
	}

	function setAcceptedClasses($classes) {
		$this->data['ACCEPT_CLASS_IDS'] = implode(', ', $classes);
		$this->_save();
	}
	
	/**
	 * Checks if a clsid assignment is inherited from parent
	 * @param	string	The clsid to check
	 * @return	boolean
	 */
	function isInheritedClass($clsid) {
		if (null != ($class = SyndNodeLib::loadClass($clsid)) && (($parent = $this->getParent()) instanceof synd_node_folder)) {
			$clsids = $parent->getClasses();
			while (null != $class && null != ($clsid2 = substr($class, 10))) {
				if (in_array($clsid2, $clsids))
					return true;
				$class = get_parent_class($class);
			}
		}
		return false;
	}

	function getClasses() {
		$clsids = array();
		if ($this->data['ACCEPT_CLASS_IDS'])
			$clsids = preg_split('/[^\w]+/', $this->data['ACCEPT_CLASS_IDS']);
		if ((($parent = $this->getParent()) instanceof synd_node_folder))
			$clsids = array_unique(array_merge($clsids, $parent->getClasses()));
		
		// Strip classes whose parent classes are already included
		foreach ($clsids as $key => $clsid) {
			if (null == ($class = SyndNodeLib::loadClass($clsid))) 
				unset($clsids[$key]);
			else {
				while (null != ($class = get_parent_class($class)) && null != ($clsid2 = substr($class, 10))) {
					if (in_array($clsid2, $clsids)) {
						unset($clsids[$key]);
						continue 2;
					}
				}
			}
		}
		
		return $clsids;
	}
	
	function getBranch() {
		$branch = array();
		$parent = $this->getParent();
		while (!$parent->isNull()) {
			$branch[$parent->nodeId] = $parent;
			$parent = $parent->getParent();
		}
		$branch = array_reverse($branch);
		$branch[$this->nodeId] = $this;
		return $branch;
	}
	
	function getFolderTree($perm = null) {
		$folders = array();
		if (null == $perm || $this->isPermitted($perm))
			$folders[$this->nodeId] = $this;
		foreach (array_keys($children = $this->getFolders()) as $key) {
			if (null == $perm || $children[$key]->isPermitted($perm)) {
				foreach (array_keys($tree = $children[$key]->getFolderTree($perm)) as $key2)
					$folders[$key2] = $tree[$key2];
			}
		}
		return $folders;
	}
	
	function validate($data) {
		$errors = parent::validate($data);
		if (isset($data['INFO_HEAD']) && null == trim($data['INFO_HEAD']))
			$errors['INFO_HEAD'] = SyndLib::translate('Please supply a title');
		return $errors;
	}
	
	function merge($data) {
		$data['ACCEPT_CLASS_IDS'] = isset($data['classes']) ?
			implode(', ', $data['classes']) : null;
		if (isset($data['PARENT_NODE_ID']) && null != ($parent = SyndNodeLib::getInstance($data['PARENT_NODE_ID']))) {
			$parent->appendChild($this);
			unset($data['PARENT_NODE_ID']);
		}
		parent::merge($data);
	}
	
	function _view_newFolder(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		$folder = $this->appendChild($this->_storage->factory('folder'));
		return array('redirect' => tpl_view_jump($folder->getHandler(),'edit',$folder->nodeId));
	}
	
	function _view_newContent(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (!isset($request['class']) || !SyndNodeLib::loadClass($request['class']) || 
			!$this->isValidClass("synd_node_".$request['class']))
			return array('content' => $page->fetchnode($this,'confirm_create_content.tpl'));
		else {
			$child = $this->appendChild(SyndNodeLib::factory($request['class']));
			return array('redirect' => tpl_view_jump($child->getHandler(),'edit',$child->nodeId));
		}
	}
	
	function _view_delOptionalValue(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		if (isset($this->data[$request[2]])) {
			$this->data[$request[2]] = null;
			$this->_node_onchange_internal();
		}
	}
	
	function _node_onchange_child() {
		$this->_children = null;
		parent::_node_onchange_child();
	}
	
	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();
		$this->data['TS_UPDATE'] = time();
		$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
	}
}
