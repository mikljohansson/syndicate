<?php
require_once 'core/module/node.class.inc';

/**
 * Project planning and time tracking module
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_plan extends synd_module_node {
	/**
	 * @access	private
	 */
	var $_title = null;

	function __construct() {
		SyndLib::attachHook('menu', array($this, '_callback_menu'));
	}
	
	/**
	 * Loads the issue.css stylesheet and displays the sidebar
	 */
	function request(Request $request) {
		SyndLib::attachHook('breadcrumbs', array($this, '_callback_breadcrumbs'));
		tpl_load_stylesheet(tpl_design_uri('module/plan/plan.css'));
		return parent::request($request);
	}
	
	function _callback_html_head_title(&$result) {
		if (null == $this->_title)
			return parent::_callback_html_head_title($result);
		return $result = $this->_title;
	}
	
	function _callback_menu(&$result, $menu) {
		if ($this->isPermitted('read'))
			$menu->append(tpl_link('plan'), SyndLib::translate('Planning'));
	}

	function _callback_breadcrumbs(&$result, Template $page) {
		$result[] = array(
			'uri' => tpl_link('plan'),
			'text' => SyndLib::translate('Projects'));

		if (null != $this->_node) {
			$crumbs = array();
			$parent = $this->_node;
			while (null != ($parent = $parent->getParent()) && !$parent->isNull()) {
				$crumbs[] = array(
					'uri' => tpl_link('plan','view',$parent->nodeId),
					'text' => $parent->toString());
			}
			$result = array_merge($result, array_reverse($crumbs));
		}
	}

	function getDescription() {
		return SyndLib::translate('Project planning and time tracking');
	}

	/**
	 * @access	protected
	 */
	function _hook_manage_permissions(&$result) {
		$result[] = $this;	
	}

	/**
	 * Returns top level projects
	 * @return	array
	 */
	function getProjects() {
		global $synd_maindb;
		$sql = "
			SELECT p.node_id FROM synd_project p
			WHERE p.parent_node_id = 'null.plan'";
		return SyndLib::filter(SyndNodeLib::getInstances($synd_maindb->getCol($sql,0)), 'isPermitted', 'read');
	}
	
	function _view_index(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		$page->assign('module', $this);
		$page->assign('projects', $this->getProjects());
		return array('content' => $page->fetch(tpl_design_path('module/plan/index.tpl')));
	}

	/**
	 * Displays the full_view.tpl of a node
	 */
	function _view_view(Request $request, Template $page) {
		if (!isset($request[0]) || null === ($this->_node = SyndNodeLib::getInstance($request[0])))
			throw new NotFoundException();

		// Determine view to display
		if (isset($request[1]) && $this->_node->isPermitted('read', "full_view_{$request[1]}.tpl"))
			$view = "full_view_{$request[1]}.tpl";
		else if ($this->_node->isPermitted('read'))
			$view = "full_view.tpl";
		else
			throw new ForbiddenException();

		// Render node
		$mres = array();
		$mres['content'] = $page->fetchnode($this->_node, 
			($this->_node instanceof synd_node_project) ? "plan/$view" : $view, 
			array('request' => $request, 'mres' => &$mres));
		return $mres;
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
		
		$_SESSION['synd']['issue']['ole'][] = $action;
	}
}
