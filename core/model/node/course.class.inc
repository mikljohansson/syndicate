<?php
require_once 'core/model/DatabaseEntity.class.inc';

class synd_node_course extends AbstractDatabaseEntity {
	var $_children = null;
	
	function __factory() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_course');
		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_CREATE'] = time();
		parent::__factory();
	}

	function __clone() {
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_course');
		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_CREATE'] = time();
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_course');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function hasPermission($user, $perm, $arg = null) {
		if ('read' == $perm && 'full_view_admin.tpl' == $arg)
			return $this->isPermitted('admin');
		if ('merge' == $perm && in_array($arg, array(
			'INFO_COURSE_ID','INFO_STYLESHEET','FLAG_STYLESHEET')))
			return $this->hasPermission($user, 'write');

		$course = Module::getInstance('course');
		if ('write' == $perm && $this->isNew() && $course->isPermitted('write'))
			return true;
		if ('admin' == $perm) {
			$creator = $this->getCreator(); 
			if ($creator->hasAuthorization($user, $this))
				return true;
		}

		// Beware of recursion as $page might defer back to $this
		if ('append' == $perm) {
			$page = $this->getPage();
			return $page->isPermitted($perm, $arg);
		}
		
		if (parent::hasPermission($user, $perm, $arg))
			return true;
			
		// Inject 'register' permission before 'read'
		if ('register' == $perm && $this->hasPermission($user, 'read', $arg))
			return true;
	}

	function getDefinedPermissions() {
		$perms = array(
			'register' => SyndLib::translate('Allows users to register to this course'),
			);
		return array_merge($perms, parent::getDefinedPermissions());
	}

	function hasAuthorization(Instance $user, Instance $subject) {
		return $this->isMember($user) || parent::hasAuthorization($user, $subject);
	}

	function getDefinedRoles() {
		return SyndLib::array_merge_assoc(array(
			'node.role_member.Registered' => SyndNodeLib::getInstance('role_member.Registered')),
			parent::getDefinedRoles());
	}

	function getParent() {
		if (null === ($parent = SyndNodeLib::getInstance($this->data['PARENT_NODE_ID'])))
			$parent = parent::getParent();
		return $parent;
	}

	function setParent($parent) {
		$this->data['PARENT_NODE_ID'] = $parent->nodeId;
		$this->_save();
	}
	
	function hasDiagnosticTest() {
		$page = $this->getPage();
		return $page->hasDiagnosticTest();
	}
	
	function hasStylesheet() {
		return $this->data['FLAG_STYLESHEET'];
	}
	
	function insertBefore($child, $before = null) {
		$page = $this->getPage();
		return $page->insertBefore($child, $before);
	}
	
	function toString() {
		$page = $this->getPage();
		return $page->toString();
	}
	
	function validate($data) {
		$errors = parent::validate($data);
		if (isset($data['INFO_COURSE_ID']) && !preg_match('/^[\w\-]*$/', $data['INFO_COURSE_ID']))
			$errors['INFO_COURSE_ID'] = SyndLib::translate('The course id may only contain letters and numbers');
		return $errors;
	}
	
	function merge($data) {
		$page = $this->getPage();
		if (isset($data['INFO_HEAD'])) {
			$page->setTitle($data['INFO_HEAD']);
			$page->data['FLAG_DIAGNOSTIC'] = isset($data['FLAG_DIAGNOSTIC']) ? 1 : 0;
		}

		if (isset($data['INFO_DESC']))
			$page->setDescription($data['INFO_DESC']);
		if (isset($data['INFO_BODY']))
			$page->setBody($data['INFO_BODY']);
		
		unset($data['INFO_HEAD'], $data['FLAG_DIAGNOSTIC'], 
			$data['INFO_DESC'], $data['INFO_BODY']);

		if (isset($data['INFO_STYLESHEET']) && !isset($data['FLAG_STYLESHEET']))
			$data['FLAG_STYLESHEET'] = 0;

		parent::merge($data);
	}
	
	function delete() {
		parent::delete();
		SyndLib::invoke($this->getChildren(), 'delete');
		
		$page = $this->getPage();
		$page->delete();

		$group = $this->getGroup();
		$group->delete();
	}

	function getCourse() {
		return $this;
	}
	
	function getCreator() {
		if (null === ($creator = SyndNodeLib::getInstance($this->data['CREATE_NODE_ID'])))
			$creator = SyndNodeLib::getInstance('user_null.null');
		return $creator;
	}
	
	/**
	 * Returns the branch to this node indexed by nodeId
	 * @return	array
	 */
	function getBranch() {
		return array($this->data['PAGE_NODE_ID'] => $this);
	}

	function getHandler() {
		return 'course';
	}
	
	function getChildren() {
		if (null === $this->_children) {
			$sql = "
				SELECT n.node_id, p.info_weight, p.info_head 
				FROM synd_node n, synd_node_page p
				WHERE 
					n.node_id = p.node_id AND
					n.parent_node_id = ".$this->_db->quote($this->nodeId)."
				ORDER BY p.info_weight, p.info_head";
			$this->_children = (array)$this->_db->getCol($sql);
			$this->_node_onchange_internal();
		}
		return $this->_storage->getInstances($this->_children);
	}
	
	function getChapters() {
		$page = $this->getPage();
		return array($page->nodeId => $page);
	}

	/**
	 * Returns the course identifier or node id.
	 * @return	string
	 */
	function getCourseId() {
		return null != $this->data['INFO_COURSE_ID'] ?
			$this->data['INFO_COURSE_ID'] : $this->nodeId;
	}
	
	/**
	 * Resolves a course identifier into a node id.
	 *
	 * Can take a course identifier or a node id and always returns
	 * a valid node id or the $id that was supplied.
	 *
	 * @param	string	Course identifier or node id
	 * @return	string
	 */
	static function resolveCourseId($id) {
		if (preg_match('/^[\w\-]+$/', $id)) {
			$database = SyndNodeLib::getDefaultStorage('course')->getDatabase();
			$sql = "
				SELECT c.node_id FROM synd_course c
				WHERE LOWER(c.info_course_id) = ".$database->quote(strtolower($id));
			if (null != ($nid = $database->getOne($sql)))
				return $nid;
		}
		return $id;
	}

	function getPage() {
		if (null == ($page = SyndNodeLib::getInstance($this->data['PAGE_NODE_ID']))) {
			$page = SyndNodeLib::factory('page');
			$page->setParent($this);
			$page->save();
			$this->data['PAGE_NODE_ID'] = $page->nodeId;
			$this->_save();
		}
		return $page;
	}
	
	function getGroup() {
		if (null == ($group = SyndNodeLib::getInstance($this->data['GROUP_NODE_ID']))) {
			$group = SyndNodeLib::factory('group');
			$group->setParent($this);
			$group->save();
			$this->data['GROUP_NODE_ID'] = $group->nodeId;
			$this->_save();
		}
		return $group;
	}
	
	function getGroups() {
		$group = $this->getGroup();
		return $group->getGroups();
	}
	
	function getTitle() {
		$page = $this->getPage();
		return $page->data['INFO_HEAD'];
	}
	
	function getDescription() {
		$page = $this->getPage();
		return $page->getDescription();
	}
	
	function getBody() {
		$page = $this->getPage();
		return $page->getBody();
	}
	
	function getRevisions() {
		$page = $this->getPage();
		return $page->getRevisions();
	}

	function getIndexDocument($index) {
		$page = $this->getPage();
		return $page->getIndexDocument($index);
	}
	
	function getPrevious() {
		return null;
	}
	
	function getNext($key = null) {
		return SyndLib::array_next($this->getChildren(), $key);
	}

	/**
	 * Returns all progress attempt on this course given a client
	 * @param	User
	 * @return	array
	 */
	function getClientAttempts($client) {
		$page = $this->getPage();
		return $page->getClientAttempts($client);
	}
	
	function getWorstAttempt($client) {
		$page = $this->getPage();
		return $page->getWorstAttempt($client);
	}
	
	function _public_style() {
		header('Cache-Control: private');
		header('Content-Type: text/css');
		return array('content' => $this->data['INFO_STYLESHEET'], 'page' => null);
	}
	
	function _createGroup($parent) {
		$group = SyndNodeLib::factory('group');
		$group->setParent($parent);
		return $group;
	}

	function createClientAccounts(Template $page, $group, $data) {
		$module = Module::getInstance('user');
		$created = array();
		$failed = array();
		$lines = array_filter(SyndLib::trim(explode("\n", $data)),'strlen');

		foreach ($lines as $line) {
			if (!preg_match('/([\w\-+\.@]+)[^\w]*(.*)/', $line, $matches)) {
				$failed[] = array($line, SyndLib::translate('Specify an email address'));
				continue;
			}

			$email = trim($matches[1]);
			if (null == ($name = trim($matches[2])))
				$name = $email;
			$password = substr(md5(uniqid('')),0,8);

			if (null != ($user = $module->resolveLogin($email))) {
				$parent = $user->getParent();
				if ('case.course' == $parent->nodeId) 
					$user->setPassword($password);
				else {
					$group->addMember($user);
					$failed[] = array($line, SyndLib::translate("Registered existing account (%s) on course", $user->toString()));

					$subject = SyndLib::translate('Account for %s', tpl_chop($this->toString(),35));
					$body = $page->fetchnode($this, 'mail_registered_instructions.tpl', array('user' => $user));
					if (!mail($email, $subject, $body, "From: noreply@{$_SERVER['SERVER_NAME']}"))
						$failed[] = array($email, SyndLib::translate('Could not send login instructions'));
					continue;
				}
			}
			else {
				$user = SyndNodeLib::factory('user');
				$data = array(
					'USERNAME' => $email,
					'PASSWORD' => $password,
					'PASSWORD2' => $password,
					'INFO_HEAD'  => $name,
					'INFO_EMAIL' => $email,);

				if (count($errors = $user->validate($data))) {
					$failed[] = array($line, implode(', ', $errors));
					continue;
				}

				$user->merge($data);
				$user->setParent(SyndNodeLib::getInstance('case.course'));
			}

			$subject = SyndLib::translate('Account for %s', tpl_chop($this->toString(),35));
			$body = $page->fetchnode($this, 'mail_client_instructions.tpl', 
				array('user' => $user, 'password' => $password));
			if (!mail($email, $subject, $body, "From: noreply@{$_SERVER['SERVER_NAME']}")) {
				$failed[] = array($email, SyndLib::translate('Could not send login instructions'));
				continue;
			}

			$user->save();
			$group->addMember($user);

			$created[] = $user;
		}
		
		return array($created, $failed);
	}
	
	function getMembers() {
		$group = $this->getGroup();
		return $group->getMembers();
	}

	function isMember($client) {
		$group = $this->getGroup();
		return $group->isMember($client);
	}
		
	function _view_register(Request $request, Template $page) {
		global $synd_user;
		Module::getInstance('course');
		if (!$this->isPermitted('register'))
			throw new ForbiddenException();
		
		if (empty($request['group_node_id'])) 
			return array('content' => $page->fetchnode($this,'confirm_registration.tpl'));
		else {
			$group = SyndNodeLib::getInstance($request['group_node_id']);
			$primary = $this->getGroup();
			if ($primary->isDescendant($group))
				$group->addMember($synd_user);
		}
		
		return array('redirect' => tpl_view($this->getHandler(),'view',$this->getCourseId()));
	}
	
	function _view_unregister(Request $request, Template $page) {
		global $synd_user;
		Module::getInstance('course');
		
		$group = $this->getGroup();
		$group->removeMemberRecursive($synd_user);
	}
	
	function _entity_onpostinsert() {
		parent::_entity_onpostinsert();
		$page = $this->getPage();
		$page->save();
	}

	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();
		$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_UPDATE'] = time();
	}
}
