<?php
require_once 'core/module/node.class.inc';

/**
 * User module
 *
 * The user module should be loaded after the cache module if 
 * possible to enable the logged in user object to be cached.
 *
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_user extends synd_module_node {
	/**
	 * @access	private
	 */
	var $_title = null;

	function __construct($config = null) {
		global $synd_maindb;
		parent::__construct($config);

		// Enforce session IP binding
		if (isset($_SERVER['REMOTE_ADDR'], $_SESSION['synd']['perm']['ip']) && 
			$_SERVER['REMOTE_ADDR'] != $_SESSION['synd']['perm']['ip']) {
			session_regenerate_id(true);
			session_unset();
			session_destroy();
		}
		
		if (isset($_SERVER['PHP_AUTH_USER'], $_SERVER['PHP_AUTH_PW'])) {
			if (false !== ($user = $this->validateLogin($_SERVER['PHP_AUTH_USER'], $_SERVER['PHP_AUTH_PW'])))
				$GLOBALS['synd_user'] = $user;
		}
		else if (isset($_SESSION['synd']['perm']['auth_node_id'])) {
			if (null !== ($user = SyndNodeLib::getInstance($_SESSION['synd']['perm']['auth_node_id'])))
				$GLOBALS['synd_user'] = $user;
			else
				unset($_SESSION['synd']['perm']['auth_node_id']);
		}

		SyndLib::attachHook('menu', array($this, '_callback_menu'));
		SyndLib::attachHook('menu_admin', array($this, '_callback_menu_admin'));
		if (empty($this->_config['hideblock']))
			SyndLib::attachHook('block', array($this, '_callback_block'), -100);
		SyndLib::attachHook('search_result', array($this, '_callback_search_result'));
	}

	/**
	 * @access	protected
	 */
	function _hook_manage_permissions(&$result) {
		$result[] = $this;	
	}
	
	function getDefinedPermissions() {
		$perms = array(
			'signup' => SyndLib::translate('Allows users to register for a user account'),
			'recover_password' => SyndLib::translate('Allows users to recover forgotten passwords'),
			);
		return array_merge($perms, parent::getDefinedPermissions());
	}

	function _callback_html_head_title(&$result) {
		if (null == $this->_title)
			return parent::_callback_html_head_title($result);
		return $result = $this->_title;
	}
	
	function _callback_menu(&$result, $menu) {
		if (isset($_SESSION['synd']['perm']['real_node_id']) && 
			null !== ($real = SyndNodeLib::getInstance($_SESSION['synd']['perm']['real_node_id'])))
			$menu->append(tpl_link('system','user','admin'), SyndLib::translate('Set user'));
	}

	function _callback_menu_admin(&$result, $menu) {
		if ($this->isPermitted('admin'))
			$menu->append(tpl_link('system','user','admin'), SyndLib::translate('Users'), '');
	}

	function _callback_block(&$result, Template $page) {
		global $synd_user;
		$block = clone $page;
		$block->assign('module', $this);
		
		if ($synd_user->isNull()) {
			$result[] = array(
				'text' => SyndLib::translate('Login'),
				'content' => $block->fetch(tpl_design_path('module/user/blocks/login.tpl')));
		}
		else {
			$block->assign('user', $synd_user);
			$result[] = array(
				'text' => SyndLib::translate('Logged in'),
				'content' => $block->fetch(tpl_design_path('module/user/blocks/logout.tpl')));
		}
	}
	
	/**
	 * Attempts to resolve usernames and telephone numbers and display the user summary
	 */
	function _callback_search_result(&$result, Request $request, Template $page, $query, $offset, $limit, $count) {
		if (0 == $offset) {
			// Match for username
			if (preg_match('/^\s*([\w\-]*[a-z][\w\-]*)\s*$/', $query, $m))
				$user = $this->resolveLogin($m[1]);
			// Match for email
			else if (preg_match('/^\s*([\w\-\.]+@[\w\-\.]+)\s*$/', $query, $m))
				$user = $this->resolveEmail($m[1]);
			// Match for number
			else if (preg_match('/^\s*(\d+)\s*$/', $query)) {
				$collection = $this->findUsers($query);
				if (count($users = $collection->getContents(0, 1)))
					$user = $users[key($users)];
				$count = $collection->getCount();
			}
				
			if (isset($user)) {
				$summary = clone $page;
				$summary->assign('user', $user);
				$summary->assign('count', isset($count) ? $count : 1);
				$summary->assign('request', $request);
				$result .= $summary->fetch(tpl_design_path('module/user/summary_search_result.tpl'));
			}
		}
	}
	
	function getDescription() {
		return SyndLib::translate('Provides user authentication, signup and services to the framework for finding users.');
	}

	/**
	 * Searches for users
	 * @param	string	Query to search for
	 * @return	Collection
	 */
	function findUsers($query) {
		return $this->_findInstances($this->_config['user_class_ids'], $query);
	}

	/**
	 * Searches for roles
	 * @param	string	Query to search for
	 * @return	Collection
	 */
	function findRoles($query) {
		return $this->_findInstances($this->_config['role_class_ids'], $query);
	}
	
	private function _findInstances($classes, $query) {
		$collections = array();
		foreach ($classes as $clsid) {
			if (null != ($collection = call_user_func(array(SyndNodeLib::loadClass($clsid), 'findInstances'), $query)))
				$collections[] = $collection;
		}
		
		switch (count($collections)) {
			case 0:
				return SyndType::factory('null_collection');
			case 1:
				return reset($collections);
		}
		
		return SyndType::factory('composite_collection', $collections);
	}

	/**
	 * Attempts to resolve a username into a user object
	 * @param	string	Username to resolve
	 * @return	User	Returns null if no user found
	 */
	function resolveLogin($login) {
		if (null != $login) {
			foreach ($this->_config['user_class_ids'] as $clsid) {
				if (null != ($user = call_user_func(array(SyndNodeLib::loadClass($clsid), 'resolveLogin'), $login)))
					return $user;
			}
		}
		return null;
	}

	/**
	 * Attempts to resolve an email into a user object
	 * @param	string		Username to resolve
	 * @return	User	Returns null if no user found
	 */
	function resolveEmail($email) {
		if (null != $email) {
			foreach ($this->_config['user_class_ids'] as $clsid) {
				if (null != ($user = call_user_func(array(SyndNodeLib::loadClass($clsid), 'resolveEmail'), $email)))
					return $user;
			}
		}
		return null;
	}
	
	function resolveRole($id) {
		if (null != $id) {
			foreach ($this->_config['role_class_ids'] as $clsid) {
				if (null != ($role = call_user_func(array(SyndNodeLib::loadClass($clsid), 'findIdentifier'), $id)))
					return $role;
			}
		}
		return null;
	}

	/**
	 * Validates a username and password and return the user object
	 * @param	string	Username to validate against
	 * @param	string	Password to validate
	 * @return	User	Returns false on failure
	 */
	function validateLogin($login, $password) {
		foreach ($this->_config['user_class_ids'] as $clsid) {
			if ((null != ($user = call_user_func(array(SyndNodeLib::loadClass($clsid), 'resolveLogin'), $login)) ||
				 null != ($user = call_user_func(array(SyndNodeLib::loadClass($clsid), 'resolveEmail'), $login))) && 
				true === $user->validatePassword($password))
				return $user;
		}
		return false;
	}
	
	function _login($user) {
		$_SESSION['synd']['perm']['auth_node_id'] = $user->nodeId;
		if (!empty($this->_config['enforce_session_ip']) && isset($_SERVER['REMOTE_ADDR']))
			$_SESSION['synd']['perm']['ip'] = $_SERVER['REMOTE_ADDR'];			
	}

	/**
	 * Authenticates a user for this session
	 *
	 * Returns the user object or false on failure
	 *
	 * @param	string	Username
	 * @param	string	Password
	 * @return	User
	 */
	function login($login, $password) {
		if (false !== ($user = $this->validateLogin($login, $password)))
			$this->_login($user);
		return $user;
	}

	/**
	 * Autosuggest callback for finding users
	 * @param	string	Query to search for
	 * @return	array
	 */
	function findSuggestedUsers($query) {
		$cid = 'user.findSuggestedUsers.'.md5($query);
		if (null == ($result = SyndLib::runHook('cache_get', $cid))) {
			$collection = $this->findUsers($query);
			$users = $collection->getContents(0, 10);
			$result = array();

			foreach (array_keys($users) as $key) {
				$name = $users[$key]->toString();
				if (null != $users[$key]->getContact())
					$name .= ' <span class="Info">('.$users[$key]->getContact().')</span>';
				if (null != $users[$key]->getEmail())
					$result[$users[$key]->getEmail()] = $name;
				else
					$result[$users[$key]->getLogin()] = $name;
			}
			
			SyndLib::runHook('cache_set', $cid, $result);
		}
		return $result;
	}
	
	/**
	 * Autosuggest clients callback
	 * @see		synd_module_user::findSuggestedUsers()
	 */
	function _public_findSuggestedUsers($query) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		return $this->findSuggestedUsers($query);
	}

	/**
	 * Autosuggest callback for finding roles
	 * @param	string	Query to search for
	 * @return	array
	 */
	function findSuggestedRoles($query) {
		$cid = 'user.findSuggestedRoles.'.md5($query);
		if (null == ($result = SyndLib::runHook('cache_get', $cid))) {
			$collection = $this->findRoles($query);
			$roles = $collection->getContents(0, 10);
			$result = array();

			foreach ($roles as $role) {
				$result[$role->getIdentifier()] = $role->getDescription() ? 
					$role->toString().' <span class="Info">('.tpl_chop($role->getDescription(),15).')</span>' : $role->toString();
			}
			
			SyndLib::runHook('cache_set', $cid, $result);
		}
		return $result;
	}

	function _view_index($request, $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();

		$page->assign('request', $request);
		$this->_title = SyndLib::translate('User directory');

		if (isset($request['q']) && '' != $request['q']) {
			$collection = $this->findUsers($request['q']);
			$count = $collection->getCount();
			
			if (!empty($request['redirect']) && 1 == $count && count($users = $collection->getContents(0, 1)))
				return array('redirect' => tpl_view('user','summary',$users[key($users)]->nodeId));
			
			$page->assign('collection', $collection);
			$page->assign('offset', isset($request['offset']) ? $request['offset'] : 0);
			$page->assign('limit', 50);
			$page->assign('count', $count);
		}

		return array('content' => $page->fetch(tpl_design_path('module/user/index.tpl')));
	}
	
	function _view_admin(Request $request, Template $page) {
		global $synd_user;
		$page->assign('title', SyndLib::translate('User administration'));

		if (!isset($_SESSION['synd']['perm']['real_node_id']))
			$real = $synd_user;
		else {
			$real = SyndNodeLib::getInstance($_SESSION['synd']['perm']['real_node_id']);
			$page->assign('real', $real);
		}
		
		if (!$this->hasPermission($real, 'admin'))
			throw new ForbiddenException();
		return array('content' => $page->fetch(tpl_design_path('module/user/admin.tpl')));
	}
	
	/**
	 * @todo	Implement persistent logins using 'variable_get' hooks
	 */
	function _view_login(Request $request, Template $page) {
		global $synd_user;
		require_once 'core/lib/SyndHTTP.class.inc';
		if (!$synd_user->isNull())
			return array('redirect' => tpl_uri_return());

		$page->assign('request', $request);
		$page->assign('title', SyndLib::translate('Login'));

		if (!isset($_COOKIE[ini_get('session.name')])) {
			$page->assign('message', SyndLib::translate("To login your webbrowser must have cookies turned on."));
			return array('content' => $page->fetch(tpl_design_path('module/user/login_error.tpl')));
		}

		if (!isset($request['username'], $request['password'])) 
			return array('content' => $page->fetch(tpl_design_path('module/user/login.tpl')));
		
		if (false == ($user = $this->login($request['username'], $request['password']))) {
			$page->assign('message', SyndLib::translate("Your username or password is incorrect."));
			return array('content' => $page->fetch(tpl_design_path('module/user/login_error.tpl')));
		}

		session_regenerate_id(true);
	}

	function _view_logout(Request $request, Template $page) {
		if (isset($_SESSION['synd']['perm']))
			unset($_SESSION['synd']['perm']['auth_node_id']); 
		session_regenerate_id(true);
		session_destroy();
	}
	
	/**
	 * User summary page which allows other modules to hook in content
	 */
	function _view_summary(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		if (!isset($request[0]) || null === ($user = SyndNodeLib::getInstance($request[0])))
			throw new NotFoundException();
		
		$page->assign('user', $user);
		$page->assign('request', $request);
		$page->assign('title', $user->toString());
		
		return array('content' => $page->fetch(tpl_design_path('module/user/summary.tpl')));
	}
	
	function _view_signup(Request $request, Template $page) {
		if (!$this->isPermitted('signup'))
			throw new ForbiddenException();
		$user = SyndNodeLib::factory('user');
		return array('redirect' => tpl_link('user','edit',$user->nodeId,'step1'));
	}
	
	function _view_edit(Request $request, Template $page) {
		$mres = parent::_view_edit($request, $page);
		if (null !== ($user = $this->getActiveNode()) && $user->isNew() && $user->isValid())
			$this->_login($user);
		return $mres;
	}
	
	/**
	 * Recover forgotten password
	 */
	function _view_password(Request $request, Template $page) {
		global $synd_maindb;

		$page->assign('request', $request);

		$storage = SyndNodeLib::getDefaultStorage('user');
		$database = $storage->getDatabase();
		
		if (isset($request['id'])) {
			$sql = "
				SELECT u.node_id FROM synd_node_user u
				WHERE u.info_persist_id = ".$database->quote($request['id']);
			$list = $storage->getInstances($database->getCol($sql));
			
			if (1 != count($list))
				$page->assign('error', SyndLib::translate('Invalid id supplied, please try again and contact the support crew if the problem persists.'));
			else {
				$user = $list[key($list)];
				$this->_login($user);
				return array('redirect' => tpl_view($user->getHandler(),'edit',$user->nodeId));
			}
		}
		else if (isset($request['username'])) {
			$sql = "
				SELECT u.node_id FROM synd_node_user u
				WHERE LOWER(u.username) = ".$database->quote(strtolower($request['username']));
			$list = $storage->getInstances($database->getCol($sql));

			if (empty($list)) {
				$sql = "
					SELECT u.node_id FROM synd_node_user u
					WHERE LOWER(u.info_email) = ".$database->quote(strtolower($request['username']));
				$list = $storage->getInstances($database->getCol($sql));
			}
			
			if (!count($list))
				$page->assign('error', SyndLib::translate('No user with that username or email found.'));
			else {
				$isValid = false;
				foreach (array_keys($list) as $key) {
					if (null != ($email = $list[$key]->getEmail())) {
						$isValid = true;
						$subject = SyndLib::translate('Forgotten password at %s',$_SERVER['SERVER_NAME']);
						$body = $page->fetch(tpl_design_path('module/user/password_mail.tpl'), array('node' => $list[$key]));
						mail($email, $subject, $body, "From: noreply@{$_SERVER['SERVER_NAME']}");
					}
				}

				if ($isValid)
					$page->assign('status', SyndLib::translate('An email with further instructions has been sent.'));
				else
					$page->assign('error', SyndLib::translate('Found no valid email address for the specified user.'));
			}
		}
	
		return array('content' => $page->fetch('module/user/password.tpl'));
	}

	function _view_su(Request $request, Template $page) {
		global $synd_user, $synd_maindb;
		
		if (!isset($_SESSION['synd']['perm']['real_node_id']))
			$real = $synd_user;
		else
			$real = SyndNodeLib::getInstance($_SESSION['synd']['perm']['real_node_id']);
		
		if (!$this->hasPermission($real, 'admin'))
			throw new ForbiddenException();
		
		if (isset($request['reset']) && isset($_SESSION['synd']['perm']['real_node_id'])) {
			$_SESSION['synd']['perm']['auth_node_id'] = $_SESSION['synd']['perm']['real_node_id'];
			unset($_SESSION['synd']['perm']['real_node_id']);
		}
		else if (!empty($request['username']) && (
			null !== ($user = $this->resolveEmail($request['username'])) || 
			null !== ($user = $this->resolveLogin($request['username'])))) {
			$_SESSION['synd']['perm']['real_node_id'] = $real->nodeId;
			$_SESSION['synd']['perm']['auth_node_id'] = $user->nodeId;
		}
	}
}
