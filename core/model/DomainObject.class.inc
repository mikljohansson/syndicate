<?php
require_once 'core/Controller.class.inc';
require_once 'core/Instance.class.inc';
require_once 'core/model/DomainEvent.class.inc';
require_once 'core/model/SyndType.class.inc';
require_once 'core/model/storage/DomainStorage.class.inc';
require_once 'core/lib/SyndNodeLib.class.inc';
require_once 'core/index/Index.class.inc';

interface DomainObject {
	/**
	 * Returns the default storage for this class
	 * @return	DomainStorage
	 */
	static function getDefaultStorage();

	/**
	 * The interfaces supported by this object
	 * @return	array
	 */
	function getInterfaces();

	/**
	 * Null-object pattern
	 * @return	bool
	 */
	function isNull();

	/**
	 * Returns this node parent or a null node if no parent.
	 * @return	DomainObject
	 */
	function getParent();
}

/**
 * @access		public
 * @package		synd.core.model
 */
abstract class AbstractDomainObject extends AbstractController implements DomainObject, Instance, Indexable {
	/**
	 * @var		string
	 */
	public $nodeId = null;

	/**
	 * Data in this entity
	 * @var		array
	 */
	public $data = null;

	/**
	 * Storage handle (autoinitialized using __get() override)
	 * @var	DomainStorage
	 */
	//public $_storage = null;
	protected $_storagedsn = null;

	/**
	 * The constructor runs after loading from storage
	 * @access	protected
	 * @param	DomainStorage	Primary storage for this node
	 * @param	array			Node data
	 */
	function __construct($storage, $data) {
		$this->_storage = $storage;
		$this->data = $data;
		$this->nodeId = $this->nodeId();
	}
	
	/**
	 * Runs after factory
	 */
	function __factory() {
		$this->nodeId = $this->nodeId();
	}
	
	/**
	 * Don't serialize the domain storage
	 */
	function __sleep() {
		if (isset($this->_storage))
			$this->_storagedsn = $this->_storage->getDSN();
		$vars = array_keys(get_object_vars($this));
		if (false !== ($key = array_search('_storage', $vars)))
			unset($vars[$key]);
		return $vars;
	}

	function __wakeup() {}
	
	/**
	 * Autoconnects the storage handle
	 */
	function __get($name) {
		if ('_storage' == $name)
			return $this->_storage = DomainStorageManager::getStorage($this->_storagedsn);
		return null;
	}

	/**
	 * Clone this object.
	 *
	 * Cloned dirty nodes will not be dirty themselves, they have to 
	 * be save() explicitly
	 *
	 * @see		DatabaseConnection::nextId()
	 * @return	DomainObject
	 */
	function copy() {
		return $this->_storage->copy($this);
	}
	
	/**
	 * @access	protected
	 * @see		DomainStorage::clone()
	 * @param	string	Newly allocated node id
	 */
	function __clone() {
		$this->nodeId = $this->nodeId();
		$this->_node_onchange_internal();
	}

	/**
	 * Returns the node id
	 * @abstract
	 * @return	string
	 */
	function nodeId() {assert(false);}

	/**
	 * Returns the global id of this node
	 * @return	string
	 */
	function id() {
		return 'node.'.$this->nodeId;
	}

	/**
	 * Returns the clsid
	 * @return	string
	 */
	function classId() {
		return substr(get_class($this), 10);
	}

	/**
	 * Returns a string version of this node
	 * @return	string
	 */
	function toString() {
		return $this->nodeId;
	}
	
	/**
	 * Returns a string version of this node
	 * @return	string
	 */
	function __toString() {
		return $this->toString();
	}

	/**
	 * Returns the node title
	 * @return	string
	 */
	function getTitle() {
		return $this->toString();
	}
	
	/**
	 * Returns a node description
	 * @return	string
	 */
	function getDescription() {
		return null;
	}
	
	function getInterfaces() {
		return AbstractDomainEvent::getImplementedInterfaces(new ReflectionObject($this));
	}

	/**
	 * Checks if this node implements an interface.
	 * @param	string	Name of class or interface
	 * @return	bool
	 */
	function isInheritedFrom($interface) {
		return ($this instanceof $interface);
	}

	function isNull() {
		return false;
	}
	
	/**
	 * Checks if the logged in user has a permission
	 * @final	Implement hasPermission() for permission checking
	 * @param	string	Permission to check {read,write,admin,...}
	 * @param	mixed	Additional argument
	 * @return	bool
	 */
	function isPermitted($perm, $arg = null) {
		global $synd_user;
		return $this->hasPermission($synd_user, $perm, $arg);
	}

	/**
	 * Checks if the a user has a permission
	 * @param	User	User to check permission for
	 * @param	string	Permission to check. {read,write,...}
	 * @param	mixed	Additional argument
	 * @return	bool
	 */
	function hasPermission($user, $perm, $arg = null) {
		if ('get' == $perm && 'nodeId' == $arg)
			return true;
		if ('invoke' == $perm && in_array($arg, array('instanceid','nodeid','objectid','classid')))
			return true;
		if ('remove' == $perm && $this->hasPermission($user, 'read'))
			return true;
		return (bool)SyndLib::runHook('permission', $this, $user, $perm, $arg);
	}

	function hasAuthorization(Instance $user, Instance $subject) {
		return !$this->isNull() && $user->nodeId == $this->nodeId && 
			$user->_storage->getNamespace() == $this->_storage->getNamespace();
	}

	/**
	 * Returns the defined permissions for this node
	 * @return	array	Array of descriptions indexed by permission
	 */
	function getDefinedPermissions() {
		return array(
			'read' => SyndLib::translate('Allows viewing the contents'),
			'write' => SyndLib::translate('Allows editing contents and options'),
			'admin' => SyndLib::translate('Allows changing permissions'),
			);
	}
	
	/**
	 * Checks if a permission inherited by default
	 */
	function isDefaultInherited($perm) {
		return true;
	}
	
	/**
	 * Returns the available roles
	 *
	 * A role is a role or relationship a user can have to this node,
	 * for example the project nodes has the 'case.Registered' role
	 * for defining project members.
	 *
	 * The result must be indexed by instance id, for example the 
	 * 'case.Registered' would be indexed by 'node.case.Registered'.
	 *
	 * @return	array	Array of nodes
	 */
	function getDefinedRoles() {
		$roles = (array)Module::runHook('defined_roles', array($this));
		if (null != ($parent = $this->getParent()) && !$parent->isNull()) {
			$roles = array_merge($parent->getDefinedRoles(), $roles);
			return SyndLib::array_kintersect($roles, 
				array_unique(SyndLib::invoke($roles, 'id')));
		}
		return $roles;
	}

	/**
	 * Return the name of this node's preferred handler module.
	 * @return	string
	 */
	function getHandler() {
		return 'node';
	}
	
	/**
	 * Returns the data in this node merged with any contained nodes.
	 *
	 * Used by the node module when displaying the edit view, it 
	 * merges this data with any submitted or template data into
	 * the $data variable sent to the edit template.
	 *
	 * @return	array
	 */
	function getCompositeData() {
		return $this->data;
	}
	
	function getParent() {
		return SyndNodeLib::getInstance('null.null');
	}
	
	/**
	 * Sets the parent of this node. 
	 *
	 * Don't use this directly, use parent->appendChild() instead.
	 *
	 * @see		appendChild()
	 * @access	protected
	 * @param	DomainObject
	 */
	function setParent($parent) {
		assert('$parent instanceof DomainObject');
	}
	
	/**
	 * Returns the parents higher up i tree
	 * @return	array
	 */
	function getBranch() {
		$result = null !== ($parent = $this->getParent()) && !$parent->isNull() ? 
			$parent->getBranch() : array();
		$result[] = $this;
		return $result;
	}
	
	/**
	 * Fires an event from this object
	 * @param	DomainEvent	Event to fire
	 */
	function raiseEvent(DomainEvent $event) {
		SyndLib::runHook('event', $this, $event);
	}
	
	/**
	 * Used by children to propagate events
	 * @access	protected
	 */
	function bubbleEvent($event) {
		if (null != ($parent = $this->getParent()))
			$parent->bubbleEvent($event);
	}
	
	/**
	 * Removes a child from this node.
	 *
	 * Returns the child on success, false on error
	 *
	 * @param	DomainObject	Child to remove
	 * @return	DomainObject
	 */
	function removeChild($child) {
		return $child;
	}
	
	/**
	 * Appends a child node.
	 * 
	 * Will remove the child from it's old parent using removeChild()
	 * and call setParent on the child. 
	 *
	 * Returns the child on success, false on error.
	 *
	 * @param	DomainObject	Child to append
	 * @return	DomainObject
	 */
	function appendChild($child) {
		if (null !== ($parent = $child->getParent()) && $parent->nodeId != $this->nodeId && 
			false === ($child = $parent->removeChild($child)))
			return false;
		$child->setParent($this);
		return $child;
	}
	
	/**
	 * Inserts a child node.
	 * 
	 * Will remove the child from it's old parent using removeChild()
	 * and call setParent on the child. 
	 *
	 * Returns the child on success, false on error.
	 *
	 * @param	DomainObject	Child to insert
	 * @param	DomainObject	Insert child immediately before this child
	 * @return	DomainObject
	 */
	function insertBefore($child, $before = null) {
		return $this->appendChild($child);
	}

	/**
	 * Returns an attribute. Attributes are inherited
	 * @return	string	Returns null if attribute not found
	 */
	function getAttribute($key) {
		if (null !== ($parent = $this->getParent()) && !$parent->isNull())
			return $parent->getAttribute($key);
		return null;
	}
	
	/**
	 * Sets an attribute
	 * @param	string	Name of attribute
	 * @param	string	Attribute value
	 */
	function setAttribute($key, $value) {}

	/**
	 * Sorts an array of DomainObjects
	 * @return	array	Returns the sorted array
	 */
	static function sort($nodes, $order) {
		if (count($nodes) > 1 && !empty($order))
			uasort($nodes, array(new DomainObjectSortHelper($order), '_callback_compare'));
		return $nodes;
	}

	/**
	 * Compare this object to another
	 * @see		strcasecmp
	 * @param	DomainObject	The object to compare to
	 * @param	array			The attributes and sort order to compare
	 * @return	int
	 */
	function compare($node, $spec) {
		if (!empty($spec)) {
			foreach ($spec as $i => $key) {
				if (is_string($key) && isset($this->data[$key], $node->data[$key]) && 0 != ($result = strcasecmp($this->data[$key], $node->data[$key])))
					return isset($spec[$i+1]) && empty($spec[$i+1]) ? -$result : $result;
			}
		}
		return 0;
	}
	
	/**
	 * Indicates that this node needs to be synced to storage
	 */
	function save() {
		$this->_storage->rewrite($this);
	}
	
	/**
	 * Flushes the storage associated with this node
	 * @see		DomainStorage::flush()
	 * @return	bool	Returns true on success, false on failure
	 */
	function flush() {
		return $this->_storage->flush();
	}

	/**
	 * Checks the node out into temporary storage
	 * @see		DomainStorage::flush()
	 */
	function checkout() {
		$this->_storage->checkout($this);
	}

	/**
	 * @deprecated
	 */
	function assign($key, $value) {
		if (isset($this->data[$key]) && ($this->data[$key] instanceof SyndType))
			$this->data[$key]->delete();
		if (($value instanceof SyndType))
			$value->setParent($this);
		$this->data[$key] = $value;
		$this->_node_onchange_internal();
	}

	/**
	 * Validates node data
	 *
	 * Checks if data can be merged into the $this->data array 
	 * without errors. Returns any error messages found.
	 *
	 * Uses isPermitted('merge', field) to determine if a specific
	 * field is allowed to be merged.
	 *
	 * @see		isPermitted()
	 * @param	array	Data to validate
	 * @return	array
	 */
	function validate($data) {
		$errors = array();
		foreach (array_keys($data) as $field) {
			if (array_key_exists($field, $this->data) && !$this->isPermitted('merge', $field, $data[$field]))
				$errors[$field] = SyndLib::translate("Field '%s' is not public", SyndLib::translate($field));
		}
		return $errors;
	}

	/**
	 * Merges values into $this->data
	 * @param	array	Data to merge
	 */
	function merge($data) {
		foreach (array_keys($data) as $field) {
			if (array_key_exists($field, $this->data))
				$this->data[$field] = $data[$field];
		}
		$this->_node_onchange_internal();
	}
	
	/**
	 * @deprecated
	 */
	function validateFlow(&$data, &$errors, $step) {
		foreach ($this->validate((array)$data) as $key => $error)
			$errors[$key] = $error;
		return empty($errors);
	}

	/**
	 * @deprecated
	 */
	function mergeFlow($data, $step) {
		$this->merge((array)$data);
	}

	function delete() {
		$this->_storage->delete($this);
	}
	
	function acceptIndexer(IndexBuilder $indexer) {}

	/**
	 * Returns the index section to store node under
	 * @return	string
	 */
	function getIndexSection() {
		return 'n.'.SyndNodeLib::getInheritedBranch($this->classId());
	}

	/**
	 * Returns the index security attributes to index node under
	 *
	 * The string is used for checking security attributes on 
	 * matches. Default encoding is a tree of top-level parent
	 * clsid + branch oids like 'project.123.456'
	 *
	 * @return	array
	 */
	function getSecurityAttributes() {
		$path = $this->objectId();
		$clsid = $this->classId();
		
		for ($parent = $this->getParent(); null !== $parent && !$parent->isNull(); $parent = $parent->getParent()) {
			$path = $parent->objectId().'.'.$path;
			$clsid = $parent->classId();
		}
		
		return array($clsid.'.'.$path);
	}

	/**
	 * Displays the edit view
	 * @param	array		Request parameters
	 * @param	Template	Page template
	 */
	function _view_edit(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
			
		// Validate/merge submitted data and checkout node to temporary storage
		if (isset($request['data'])) {
			$data = $request['data'];
			if (!empty($request->files['data']))
				$data = SyndLib::array_merge_recursive($data, $request->files['data']);
		
			if (!count($errors = $this->validate($data))) {
				$this->checkout();
				$this->merge($data);
			}
		}

		if (isset($request['post']) && empty($errors)) {
			$this->save();
			return array('redirect' => tpl_uri_return(tpl_view($this->getHandler(),'view',$this->nodeId())));
		}

		if (!empty($request['redirect']) && empty($errors))
			return array('redirect' => $request['redirect']);
		
		// Determine view to display
		$view = 'full_edit.tpl';
		if (isset($request[0]) && $this->isPermitted('read',"full_edit_{$request[0]}.tpl")) {
			$view = "full_edit_{$request[0]}.tpl";
			$request = $request->forward();
		}

		$params = array(
			'node' => $this,
			'view' => $view,
			'data' => $this->getCompositeData());
		if (!empty($errors))
			$params['errors'] = $errors;

		// Merge original node data and submitted data
		if (!empty($request['data']))
			$params['data'] = SyndLib::array_merge_recursive($params['data'], $request['data']);
		if (!empty($request->files['data']))
			$params['data'] = SyndLib::array_merge_recursive($params['data'], $request->files['data']);

		$page->assign('request', $request);
		return array('content' => $page->fetch('model/node/edit.tpl', $params));
	}

	/**
	 * Node has changed in persistent storage
	 * @access	protected
	 */
	function _node_onchange() {
		$this->_node_onchange_child();
		//$this->_storage->refresh($this);
		$this->raiseEvent(new synd_event_update($this));
	}

	/**
	 * Child node has changed in persistent storage
	 * @access	protected
	 */
	function _node_onchange_child() {
		if (null !== ($parent = $this->getParent()) && !$parent->isNull())
			$parent->_node_onchange_child();
		$this->_storage->refresh($this);
	}
	
	/**
	 * Internal state has changed and needs to be synced to intermediate storage {cache, temporary, ..}
	 * @access	protected
	 */
	function _node_onchange_internal() {
		$this->_storage->refresh($this);
	}

	/**
	 * Runs before deleting from storage
	 *
	 * Returning true from this callback will rollback the running 
	 * transaction
	 *
	 * @access	protected
	 * @return	bool	
	 */
	function _node_onpredelete() {
		$parent = $this->getParent();
		$parent->removeChild($this);
		$parent->_node_onchange_child();
	}

	function _node_onpostdelete() {
		$this->raiseEvent(new synd_event_delete($this));
	}
}

/**
 * Iterates over a domain object tree structure
 * @access		public
 * @package		synd.core.model
 */
class RecursiveDomainObjectIterator implements Iterator {
	protected $_parent = null;
	protected $_decider = null;
	protected $_current = null;
	protected $_stack = null;
	
	function __construct(DomainObject $parent, MatchDecider $decider = null) {
		$this->_parent = $parent;
		$this->_decider = null != $decider ? $decider : new BooleanDecider();
	}
	
	function current() {
		return $this->_current;
	}
	
	function key() {
		return $this->_current->nodeId;
	}
	
	function next() {
		$this->_current = null;
		
		if (false != ($children = end($this->_stack))) {
			do {
				if ($children->valid()) {
					if ($this->_decider->accept($children->current()))
						$this->_current = $children->current();
					$children->next();
				}
				else {
					array_pop($this->_stack);
					$children = end($this->_stack);
				}
			} while (null == $this->_current && false != $children);
		}
		
		if (null != $this->_current) {
			$children = $this->_current->getChildren()->getIterator();
			$children->rewind();
			$this->_stack[] = $children;
		}
	}
	
	function rewind() {
		if ($this->_decider->accept($this->_parent)) {
			$this->_current = $this->_parent;
			$this->_stack = array($this->_current->getChildren()->getIterator());
		}
	}
	
	function valid() {
		return null != $this->_current;
	}
}
