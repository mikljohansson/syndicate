<?php
require_once 'core/module/node.class.inc';

/**
 * Learning management module
 *
 * Provides integration for courses and allows students to register
 * to them. Handles setup of external accounts for people without a
 * regular one.
 *
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_course extends synd_module_node {
	var $_oleTarget = null;
	
	function __construct() {
		SyndLib::attachHook('menu', array($this, '_callback_menu'));
		SyndLib::attachHook('menu_admin', array($this, '_callback_menu_admin'));
		SyndLib::attachHook('index_active_clsids', array($this, '_callback_index_active_clsids'));
		SyndLib::attachHook('index_security_attributes', array($this, '_callback_index_security_attributes'));
		SyndLib::attachHook('default_request_handler', array($this, '_callback_default_request_handler'));
	}

	/**
	 * Load logged in user and attach display hooks
	 */
	function request(Request $request) {
		SyndLib::attachHook('block', array($this, '_callback_block'));
		tpl_load_stylesheet(tpl_design_uri('module/course/course.css'));
		$result = parent::request($request);

		// Load course stylesheet
		$node = $this->getActiveNode();
		while (!$node->isNull() && !($node instanceof synd_node_course))
			$node = $node->getParent();
		if (($node instanceof synd_node_course) && $node->hasStylesheet())
			tpl_load_stylesheet(tpl_link('node','invoke',$node->nodeId,'style').'index.css');
		
		return $result;
	}

	function getDescription() {
		return SyndLib::translate('Learning management module.');
	}

	function _callback_menu(&$result, $menu) {
		if ($this->isPermitted('read'))
			$menu->append(tpl_link('course'), SyndLib::translate('Courses'), SyndLib::translate('Courses and learning management'), array($this, '_callback_menu_default'), array('','course','course/*'));
	}
	
	function _callback_menu_admin(&$result, $menu) {
		if ($this->isPermitted('write'))
			$menu->append(tpl_link('system','course','admin'), SyndLib::translate('Courses'), '', array($this, '_callback_menu_admin_default'), array('system/course','system/course/*'));
	}
	
	function _callback_menu_default($menu) {
		global $synd_user;
		foreach ($this->getClientCourses($synd_user) as $course)
			$menu->append(tpl_link('course','view',$course->getCourseId()), tpl_chop($course->toString(),20), tpl_chop($course->getDescription(),50));
	}

	function _callback_menu_admin_default($menu) {
		$menu->append(tpl_link('system','course','new'), SyndLib::translate('Create course'), '');
	}

	function _callback_default_request_handler(&$result) {
		if ($this->isPermitted('read')) {
			$result = $this;
			return true;
		}
	}
	
	/**
	 * @access	protected
	 */
	function _hook_manage_permissions(&$result) {
		$result[] = $this;	
	}
	
	function _hook_design_front_links(&$result) {
		global $synd_user;
		if ($synd_user->isNull()) 
			return;

		profiler_enter(__METHOD__);
		foreach (array_keys($courses = $this->getClientCourses($synd_user)) as $key) {
			$result[] = array(
				'uri' => tpl_view('course','view',$courses[$key]->getCourseId()),
				'text' => $courses[$key]->getTitle(),
				'title' => $courses[$key]->getDescription());
		}
		profiler_leave(__METHOD__);
	}
	
	function _callback_index_active_clsids(&$result) {
		$result[] = 'page';
		$result[] = 'question';
	}

	function _callback_index_security_attributes(&$result, $user) {
		foreach (array_keys($courses = $this->getClientCourses($user[0])) as $key) {
			if ($courses[$key]->hasPermission($user[0], 'read'))
				$result[] = $courses[$key]->getSecurityContext();
		}
	}

	function _callback_block(&$result, Template $page) {
		global $synd_user;
		
		// Render index block
		$node = $this->getActiveNode();
		$course = $node;
		while (!$course->isNull() && !($course instanceof synd_node_course))
			$course = $course->getParent();
		
		if (($course instanceof synd_node_course) && !$course->isNew() && $course->isPermitted('read')) {
			if (($node instanceof synd_node_page) || ($node instanceof synd_node_question))
				$branch = SyndLib::filter($node->getBranch(), 'isPermitted', 'read');
			else
				$branch = array();
				
			$result[] = array('content' => 
				$page->fetchnode($course,'list_view_expanded.tpl',
				array('branch' => $branch)));
		}

		// Render ole blocks
		if (isset($_SESSION['synd']['page']['ole'])) {
			foreach (array_keys($_SESSION['synd']['page']['ole']) as $key) {
				if ($_SESSION['synd']['page']['ole'][$key]->isCancelled()) {
					unset($_SESSION['synd']['page']['ole'][$key]);
					continue;
				}
				
				$target = $this->getOleTarget();
				$result[] = array('content' => 
					$page->fetchnode($_SESSION['synd']['page']['ole'][$key],'full_view.tpl',
					array('target' => $target)));
			}
		}
	}

	function getOleTarget() {
		if (null != $this->_oleTarget)
			return $this->_oleTarget;
		return $this->getActiveNode();
	}
	
	function setOleTarget($node) {
		$this->_oleTarget = $node;
	}
	
	function getClientCourses($client) {
		global $synd_maindb;
		$sql = "
			SELECT g.node_id FROM synd_group g, synd_relation r
			WHERE
				r.parent_node_id LIKE 'group.%' AND
				r.parent_node_id = g.node_id AND
				r.child_node_id = '$client->nodeId'";
				
		$groups = SyndNodeLib::getInstances($synd_maindb->getCol($sql,0));
		$courses = array();
		
		foreach (array_keys($groups) as $key) {
			if (null != ($course = $this->_findParent($groups[$key], 'synd_node_course')) &&
				!isset($courses[$course->nodeId]))
				$courses[$course->nodeId] = $course;
		}
		
		return $courses;
	}
	
	function _findParent($node, $class) {
		if (null == ($parent = $node->getParent()) || $parent->isNull())
			return null;
		if (get_class($parent) == $class)
			return $parent;
		return $this->_findParent($parent, $class);
	}
	
	function getGroups() {
		$storage = SyndNodeLib::getDefaultStorage('group');
		$sql = "
			SELECT g.node_id FROM synd_group g
			WHERE g.parent_node_id = 'case.course'";
		return $storage->getInstances($storage->getDatabase()->getCol($sql));
	}
	
	function _view_index(Request $request, Template $page) {
		global $synd_user;
		$this->_title = SyndLib::translate('Learning Managment');
		
		$storage = SyndNodeLib::getDefaultStorage('course');
		$database = $storage->getDatabase();
		
		$myCourses = $this->getClientCourses($synd_user);
		$ids = SyndLib::collect($myCourses, 'nodeId');
		
		$sql = "SELECT c.node_id FROM synd_course c";
		$courses = $storage->getInstances($database->getCol($sql));
		foreach (array_keys($courses) as $key) {
			if (in_array($courses[$key]->nodeId, $ids) || !$courses[$key]->isPermitted('read') && !$courses[$key]->isPermitted('register'))
				unset($courses[$key]);
		}
		
		$page->assign('courses', $courses);
		$page->assign('myCourses', $myCourses);
		return array('content' => $page->fetch(tpl_design_path('module/course/index.tpl')));
	}
	
	function _view_admin(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		$this->_title = SyndLib::translate('Course administration');
		return array('content' => $page->fetch(tpl_design_path('module/course/admin.tpl')));
	}

	/**
	 * Resolves course identifier into node ids.
	 * @param	array	Module view parameters
	 * @return	array
	 */
	function _view_view(Request $request, Template $page) {
		SyndNodeLib::loadClass('course');
		$request[0] = synd_node_course::resolveCourseId($request[0]);
		return parent::_view_view($request, $page);
	}
	
	/**
	 * Creates a new course
	 */
	function _view_new(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		$course = SyndNodeLib::factory('course');
		return array('redirect' => tpl_view($course->getHandler(),'edit',$course->nodeId));
	}
	
	function _view_search(Request $request, Template $page) {
		require_once 'core/db/SyndDBLib.class.inc';
		$this->_title = SyndLib::translate('Search');
		
		$page->assign('request', $request);
		
		$section = isset($request['section']) ? $request['section'] : 'page,question';
		$page->assign('section', $section);

		if (isset($request['filter'])) {
			$limit = 50;
			$offset = isset($_REQUEST['offset']) ? $_REQUEST['offset'] : 0;
			$filter = array();
			
			$cols = array(
				'q.node_id','q.update_node_id','q.info_head',
				'q.info_answer','q.info_explanation');

			if (!empty($request['query']))
				$filter[] = SyndDBLib::sqlLikeExpr($request['query'], $cols);

			if (isset($request['filter']['progress']))
				$filter[] = 'q.flag_progress = 1';
			if (isset($request['filter']['diagnostic']))
				$filter[] = 'q.flag_diagnostic = 1';


			if (count($filter)) {
				global $synd_maindb;
				$filter = implode(' AND ', $filter);

				$sql = "
					SELECT COUNT(DISTINCT q.node_id) 
					FROM synd_question q
					WHERE ($filter)";
				$count = $synd_maindb->getOne($sql);

				$sql = "
					SELECT ".implode(',',$cols)." 
					FROM synd_question q
					WHERE ($filter)";
				
				$sql = "SELECT DISTINCT q2.node_id FROM ($sql) q2";
				$matchSet = SyndNodeLib::getInstances($synd_maindb->getCol($sql, 0, $offset, $limit));
				$matchSet = SyndLib::filter($matchSet, 'isPermitted', 'read');
			
				$page->assign('matchSet', $matchSet);			
				$page->assign('limit', $limit);
				$page->assign('offset', $offset);
				$page->assign('count', $count);
			}
		}
		else if (isset($request['query'])) {
			$this->_title .= ': '.tpl_chop(strip_tags($request['query']), 15);
				
			$limit = 20;
			$offset = isset($_REQUEST['offset']) ? $_REQUEST['offset'] : 0;
			$collections = array();
			
			foreach (explode(',',$section) as $clsid) {
				if (null != ($collection = SyndLib::runHook('search', $clsid, $request['query'])))
					$collections[] = $collection;
			}
			
			if (!empty($collections)) {
				$collection = SyndType::factory('composite_collection', $collections);
				$mset = SyndLib::filter($collection->getContents($offset, $limit), 'isPermitted', 'read');
				
				if (isset($request['redirect']) && 1 == $collection->getCount() && !empty($mset)) 
					return array('redirect' => tpl_view('node','view',$mset[key($mset)]->nodeId));

				$page->assign('matchSet', $mset);
				$page->assign('count', $collection->getCount());

				$page->assign('limit', $limit);
				$page->assign('offset', $offset);
				$page->assign('highlight', SyndDBLib::parseSearchString(preg_replace('/[^\w\s]/','',$request['query'])));
			}
		}
		
		return array('content' => $page->fetch(tpl_design_path('module/course/search.tpl')));
	}
		
	function _getCollection($request) {
		if (empty($request['collections']))
			return null;

		if (count($request['collections']) <= 1) 
			$collection = SyndLib::getInstance(SyndLib::reset($request['collections']));
		else {
			$collection = SyndType::factory('composite_collection');
			foreach ((array)$request['collections'] as $id)
				$collection->appendCollection(SyndLib::getInstance($id));
		}
	
		return $collection;
	}
	
	function _view_cut(Request $request, Template $page) {
		if (null == ($collection = $this->_getCollection($request)))
			return null;

		$action = SyndType::factory('cut');
		foreach (array_keys($contents = $collection->getContents()) as $key) {
			if (!$contents[$key]->isPermitted('write'))
				throw new ForbiddenException();
			$action->appendChild($contents[$key]);
		}
		
		$_SESSION['synd']['page']['ole'][] = $action;
	}
	
	function _view_copy(Request $request, Template $page) {
		if (null == ($collection = $this->_getCollection($request)))
			return null;
				
		$action = SyndType::factory('cut');
		foreach (array_keys($contents = $collection->getContents()) as $key) {
			if (!$contents[$key]->isPermitted('read'))
				throw new ForbiddenException();
			$action->appendChild($contents[$key]);
		}
		
		$_SESSION['synd']['page']['ole'][] = $action;
	}

	function _view_delete(Request $request, Template $page) {
		$collection = $this->_getCollection($request);
		if (null != $collection) 
			$contents = $collection->getContents();
		else if (!empty($request[0]))
			$contents[] = SyndNodeLib::getInstance($request[0]);
		else
			return null;

		if (false !== array_search(false, SyndLib::invoke($contents,'isPermitted','write')))
			throw new ForbiddenException();
		
		if (isset($request['post'])) {
			foreach (array_keys($contents) as $key)
				$contents[$key]->delete();

			if (isset($request['redirect']))
				return array('redirect' => $request['redirect']);
		}
		else {
			$page->assign('contents', $contents);
			$page->assign('collection', $collection);
			return array('content' => $page->fetch('module/course/delete.tpl'));
		}
	}
}
