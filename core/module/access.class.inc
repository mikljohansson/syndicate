<?php
require_once 'core/Module.class.inc';

/**
 * Access control module
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_access extends Module {
	protected $_db = null;

	private $_permitted = array();
	private $_inherited = array();
	private $_assignment = array();
	private $_merged = array();

	/**
	 * @access	private
	 */
	var $_title = null;

	function __construct() {
		global $synd_maindb;
		$this->_db = $synd_maindb;
		SyndLib::attachHook('grant', array($this, '_callback_grant'));
		SyndLib::attachHook('permission', array($this, '_callback_permission'));
		SyndLib::attachHook('menu_admin', array($this, '_callback_menu_admin'));
	}

	function request(Request $request) {
		SyndLib::attachHook('html_head_title', array($this, '_callback_html_head_title'), -100);
		return parent::request($request);
	}
	
	function _callback_menu_admin(&$result, $menu) {
		if ($this->isPermitted('admin'))
			$menu->append(tpl_link('system','access'), SyndLib::translate('Access'));
	}

	function _callback_html_head_title(&$result) {
		return $result = $this->_title;
	}
	
	function _view_index(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		$page->assign('title', SyndLib::translate('Access control'));
		$page->assign('request', $request);
		return array('content' => $page->fetch(tpl_design_path('module/access/index.tpl')));
	}
	
	/**
	 * Checks is a user/role has a permission on a subject
	 *
	 * Permission results are cached without regard to the $arg 
	 * optional argument so it must not be distinct.
	 *
	 * @param	Instance	User or role to check permission for
	 * @param	Instance	The subject to check access to
	 * @param	string		The permission to check {read, write, admin, ...}
	 * @param	mixed		Optional token to hasPermission()
	 * @access	protected
	 */
	function checkPermission(Instance $subject, Instance $role, $perm, $arg = null) {
		$uid = $role->id();
		
		if (!isset($this->_permitted[$uid])) {
			$cid = "access_permitted_$uid";
			if (null === ($this->_permitted[$uid] = SyndLib::runHook('cache_get', $cid))) {
				try {
					$sql = "
						SELECT CONCAT(a.node, CONCAT('/',a.perm)), 1 FROM synd_access a
						WHERE a.role = ".$this->_db->quote($uid);
					$this->_permitted[$uid] = $this->_db->getAssoc($sql);
					SyndLib::runHook('cache_set', $cid, $this->_permitted[$uid]);
				}
				catch (DatabaseException $e) {
					trigger_error($e->getMessage(), E_USER_WARNING);
					return false;
				}
			}
		}

		$operation = $subject->id().'/'.$perm;
		if (isset($this->_permitted[$uid][$operation]))
			return false !== $this->_permitted[$uid][$operation];

		// If the permission is inheritable, bubble to parent
		if ($this->isInherited($subject, $perm) && null !== ($parent = $subject->getParent()) && 
			!$parent->isNull() && $parent->hasPermission($role, $perm, $arg))
			return $this->_permitted[$uid][$operation] = true;

		// Checks if the user is authorized for any of the roles that has this permission. 
		foreach (array_keys($roles = $this->getPermissionAssignment($subject, $perm)) as $key) {
			if ($roles[$key]->hasAuthorization($role, $subject))
				return $this->_permitted[$uid][$operation] = true;
		}
		
		// write = {read, write}, admin = {read, write, admin}
		if ('read' == $perm)
			return $this->_permitted[$uid][$operation] = $subject->hasPermission($role, 'write', $arg);
		if ('write' == $perm)
			return $this->_permitted[$uid][$operation] = $subject->hasPermission($role, 'admin', $arg);

		// Default admin perm to access module admins
		if ('admin' == $perm && ($subject instanceof Module) && !($subject instanceof synd_module_access))
			return $this->_permitted[$uid][$operation] = $this->hasPermission($role, $perm, $arg);

		return false;
	}

	/**
	 * Implementation of the grant permission hook
	 */
	function _callback_grant(&$result, $subject, $role, $perm, $arg = null) {
		return $result = $this->grantPermission($subject, $role, $perm);
	}
	
	/**
	 * Implementation of the check permission hook
	 */
	function _callback_permission(&$result, Instance $subject, Instance $role, $perm, $arg = null) {
		profiler_enter(__METHOD__);
		$result = $this->checkPermission($subject, $role, $perm, $arg);
		profiler_leave(__METHOD__);
		return $result;
	}
	
	/**
	 * Checks if a permission grant is inherited from parent
	 * @access	protected
	 * @param	Instance	Subject of query
	 * @param	string	Permission to check inheritance for
	 * @return	bool
	 */
	function isInherited(Instance $subject, $perm) {
		$sid = $subject->id();
		
		if (!isset($this->_inherited[$sid])) {
			profiler_enter(__METHOD__);
			
			$cid = "access_inherit_$sid";
			if (null === ($this->_inherited[$sid] = SyndLib::runHook('cache_get', $cid))) {
				try {
					$sql = "
						SELECT i.perm, i.inherit FROM synd_access_inherit i
						WHERE i.node = ".$this->_db->quote($sid);
					$this->_inherited[$sid] = $this->_db->getAssoc($sql);
					SyndLib::runHook('cache_set', $cid, $this->_inherited[$sid]);
				}
				catch (DatabaseException $e) {
					profiler_leave(__METHOD__);
					return false;
				}
			}

			profiler_leave(__METHOD__);
		}
		
		return !empty($this->_inherited[$sid][$perm]) || 
			(!isset($this->_inherited[$sid][$perm]) && $subject->isDefaultInherited($perm));
	}

	/**
	 * Returns the roles granted a permission on a subject
	 * @access	protected
	 * @param	Instance	Subject of query
	 * @param	string	Permission to get assigned roles for
	 * @return	bool
	 */
	function getPermissionAssignment(Instance $subject, $perm) {
		$sid = $subject->id();

		// Get subject specific assignments
		if (!isset($this->_assignment[$sid])) {
			profiler_enter(__METHOD__);
			$cid = "access_assignment_$sid";

			try {
				if (null !== ($buffer = SyndLib::runHook('cache_get', $cid))) {
					foreach ($buffer as $key => $roles)
						$this->_assignment[$sid][$key] = SyndLib::getInstances($roles);
				}
				else {
					$sql = "
						SELECT a.PERM, a.ROLE FROM synd_access a
						WHERE a.node = ".$this->_db->quote($sid);
					$rows = $this->_db->getAll($sql);
					$roles = SyndLib::getInstances(array_unique(SyndLib::array_collect($rows, 'ROLE')));
					$buffer = array();

					foreach ($rows as $row) {
						if (isset($roles[$row['ROLE']])) {
							$this->_assignment[$sid][$row['PERM']][$row['ROLE']] = $roles[$row['ROLE']];
							$buffer[$row['PERM']][] = $row['ROLE'];
						}
					}

					SyndLib::runHook('cache_set', $cid, $buffer);
				}
			}
			catch (DatabaseException $e) {
				profiler_leave(__METHOD__);
				return array();
			}

			profiler_leave(__METHOD__);
		}

		// Merge inherited assignments
		if ($this->isInherited($subject, $perm) && null != ($parent = $subject->getParent()) && !$parent->isNull()) {
			return isset($this->_assignment[$sid][$perm]) ? SyndLib::array_merge_assoc($this->_assignment[$sid][$perm], 
				$this->getPermissionAssignment($parent, $perm)) : 
				$this->getPermissionAssignment($parent, $perm);
		}
		
		if (isset($this->_assignment[$sid][$perm]))
			return $this->_assignment[$sid][$perm];
		
		return array();
	}
	
	/**
	 * Checks if a role permission assignment is inherited from parent
	 * @access	protected
	 * @param	Instance	Subject of query
	 * @param	string	Permission to check inheritance for
	 * @param	Instance	Role of query
	 * @return	bool
	 */
	function isInheritedAssignment(Instance $subject, $perm, Instance $role) {
		return !isset($this->_assignment[$subject->id()][$perm][$role->id()]);
	}

	/**
	 * Adds the Anonymous and Authenticated default roles
	 */
	function _hook_defined_roles(&$result, $subject) {
		$result = array_merge((array)$result, array(
			'node.role_authenticated.Authenticated' => SyndNodeLib::getInstance('role_authenticated.Authenticated'),
			'node.role_anonymous.Anonymous' => SyndNodeLib::getInstance('role_anonymous.Anonymous'),
			));
	}
	
	/**
	 * Implentation of the edit permissions hook
	 */
	function _hook_permissions(&$result, Template $page, $subject) {
		if (!$subject->isPermitted('admin'))
			return;
		$page->assign('module', $this);
		$page->assign('subject', $subject);
		$result .= $page->fetch(tpl_design_path('module/access/permissions.tpl'));
	}
	
	/**
	 * @access	protected
	 */
	function _hook_manage_permissions(&$result) {
		$result[] = $this;	
	}

	function getDescription() {
		return SyndLib::translate('Implements the access control hooks.');
	}
	
	/**
	 * Grant a role a permission on a subject
	 * @param	Instance	The subject of the grant
	 * @param	Instance	The role to grant a permission
	 * @param	string		Permission to grant
	 * @return	bool		True on success, false on failure
	 */
	function grantPermission(Instance $subject, Instance $role, $perm) {
		$sql = "
			INSERT INTO synd_access
			(role, node, perm)
			VALUES
			(".$this->_db->quote($role->id()).",
			 ".$this->_db->quote($subject->id()).",
			 ".$this->_db->quote($perm).")";
		$this->_db->query($sql);
		$this->_clear($subject);
		$this->_clearPermissions($role);
		return true;
	}
	
	function _view_savePermissions(Request $request, Template $page) {
		global $synd_user;
		if (null == ($subject = SyndLib::getInstance($request[0])))
			throw new NotFoundException();
		if (!$subject->isPermitted('admin'))
			throw new ForbiddenException();
		if (!isset($request['post']))
			return null;
		
		try {
			$this->_db->begin();
			$this->_clear($subject);
			$this->_clearTouchedPermissions();

			$sql = "
				DELETE FROM synd_access
				WHERE node = ".$this->_db->quote($subject->id());
			$this->_db->query($sql);

			if (!empty($request['perms'])) {
				foreach ((array)$request['perms'] as $perm => $roles) {
					foreach (SyndLib::getInstances($roles) as $role) {
						if (!$this->grantPermission($subject, $role, $perm)) {
							$this->_db->rollback();
							return null;
						}
					}
				}
			}

			if (!empty($request['assign']['perm']) && !empty($request['assign']['roles'])) {
				foreach (SyndLib::getInstances($request['assign']['roles']) as $role) {
					if (!$this->grantPermission($subject, $role, $request['assign']['perm'])) {
						$this->_db->rollback();
						return null;
					}
				}
			}

			$sql = "
				DELETE FROM synd_access_inherit
				WHERE node = ".$this->_db->quote($subject->id());
			$this->_db->query($sql);

			unset($this->_inherited[$subject->id()]);

			foreach (array_keys($subject->getDefinedPermissions()) as $perm) {
				$inherit = isset($request['inherit']) && in_array($perm, $request['inherit']);
				if ($subject->isDefaultInherited($perm) != $inherit) {
					$sql = "
						INSERT INTO synd_access_inherit
						(node, perm, inherit)
						VALUES
						(".$this->_db->quote($subject->id()).",
						 ".$this->_db->quote($perm).", 
						 ".($inherit ? 1 : 0).")";
					$this->_db->query($sql);
				}
			}

			// Check that user hasn't removed the admin permission from himself
			if ($subject->isPermitted('admin'))
				$this->_db->commit();
			else {
				$this->_db->rollback();
				$this->_clear($subject);
				$this->_clearTouchedPermissions();
			}
		}
		catch (RuntimeException $e) {
			$this->_db->rollback();
			throw $e;
		}
	}
	
	function _view_removePermission(Request $request, Template $page) {
		global $synd_user;
		if (null == ($subject = SyndLib::getInstance($request[0])))
			throw new NotFoundException();
		if (!$subject->isPermitted('admin'))
			throw new ForbiddenException();
		if (empty($request['role']) || empty($request['perm']))
			return;

		$this->_db->begin();
		$this->_clear($subject);
		$this->_clearTouchedPermissions();

		$sql = "
			DELETE FROM synd_access
			WHERE 
				role = ".$this->_db->quote($request['role'])." AND
				node = ".$this->_db->quote($subject->id())." AND
				perm = ".$this->_db->quote($request['perm']);
		$this->_db->query($sql);
		
		// Check that user hasn't removed the admin permission from himself
		if ($subject->isPermitted('admin'))
			$this->_db->commit();
		else {
			$this->_db->rollback();
			$this->_clear($subject);
			$this->_clearTouchedPermissions();
			return array('content' => $page->fetch(tpl_design_path('module/access/confirm_remove_admin.tpl'),
				array('subject' => $subject, 'role' => SyndLib::getInstance($request['role']))));
		}
	}
	
	/**
	 * Finds roles and users given a query
	 * @access	protected
	 * @return	Collection
	 */
	function findRoles($query) {
		$module = Module::getInstance('user');
		$roles = $module->findRoles($query);
		$users = $module->findUsers($query);
		return SyndType::factory('composite_collection', array($roles, $users));
	}

	/**
	 * Clears the permission caches for a subject
	 */
	private function _clear(Instance $subject) {
		if (null !== $subject) {
			unset($this->_assignment[$subject->id()]);
			unset($this->_inherited[$subject->id()]);

			SyndLib::runHook('cache_del', "access_assignment_".$subject->id());
			SyndLib::runHook('cache_del', "access_inherit_".$subject->id());

			$sql = "
				SELECT a.role FROM synd_access a
				WHERE a.node = ".$this->_db->quote($subject->id());
			foreach (SyndLib::getInstances($this->_db->getCol($sql)) as $role) 
				$this->_clearPermissions($role);
		}
	}
	
	/**
	 * Clears the permission caches for a role
	 */
	private function _clearPermissions(Instance $role) {
		if (null !== $role) {
			unset($this->_permitted[$role->id()]);
			SyndLib::runHook('cache_del', "access_permitted_".$role->id());
		}
	}

	/**
	 * Clears all permissions in the current cache
	 */
	private function _clearTouchedPermissions() {
		foreach (array_keys($this->_permitted) as $rid)
			$this->_clearPermissions(SyndLib::getInstance($rid));
	}

	function _view_newRole(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		$role = SyndNodeLib::factory('role');
		return array('redirect' => tpl_view('node','edit',$role->nodeId));
	}
}

