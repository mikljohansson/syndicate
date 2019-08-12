<?php
require_once 'core/Plugin.class.inc';
require_once 'core/model/DomainObject.class.inc';
require_once 'core/model/DomainEvent.class.inc';

/**
 * Workflow plugin for the issue module
 * @access		protected
 * @package		synd.core.module
 */
class synd_plugin_workflow extends Plugin {
	protected $_db = null;
	
	function __construct($config = null) {
		parent::__construct($config);
		SyndLib::attachHook('event', array($this, '_callback_event'));
		SyndLib::attachHook('project_admin', array($this, '_callback_project_admin'));
		SyndLib::attachHook('workflow_event_protos', array($this, '_callback_workflow_event_protos'));
		SyndLib::attachHook('workflow_activity_protos', array($this, '_callback_workflow_activity_protos'));
		SyndLib::attachHook('issue_block', array($this, '_callback_issue_block'));
		SyndLib::attachHook('issue_context', array($this, '_callback_issue_context'));
		SyndLib::attachHook('issue_context_menu', array($this, '_callback_issue_context_menu'));
		SyndLib::attachHook('issue_context_apply', array($this, '_callback_issue_context_apply'));
		SyndLib::attachHook('instance', array($this, '_callback_instance'), -2500);
	}
	
	function _callback_instance(&$result, $id) {
		if ('workflow' === $id) {
			$result = $this;
			return true;
		}
	}
	
	function _callback_event(&$result, DomainObject $context, DomainEvent $event) {
		$storage = SyndNodeLib::getDefaultStorage('workflow');
		$database = $storage->getDatabase();
		
		$contexts = array();
		for ($parent = $event->getSubject(); !$parent->isNull(); $parent = $parent->getParent())
			$contexts[] = $parent->nodeId;
		$interfaces = $event->getInterfaces();
		
		$sql = "
			SELECT DISTINCT w.node_id FROM synd_workflow w, synd_workflow_event e
			WHERE
				w.node_id = e.parent_node_id AND
				w.parent_node_id IN (".implode(',',$database->quote($contexts)).") AND
				e.event IN (".implode(',',$database->quote($interfaces)).")";
		$workflows = $storage->getInstances($database->getCol($sql));
		
		foreach ($workflows as $workflow)
			$workflow->handleEvent($event);
	}

	function _callback_project_admin(&$result, $node, $request, $pane) {
		$selected = isset($request[0]) && 'workflows' == $request[0];
		$tab = DefaultTemplate::factory('plugin/workflow/pane.tpl', $pane);
		
		if ($selected) {
			$tab->assign('plugin', $this);
			$tab->assign('project', $node);
			$tab->assign('limit', 25);
			$tab->assign('order', tpl_sort_order('workflow'));
			$tab->assign('offset', isset($request['offset']) ? $request['offset'] : 0);
			$tab->assign('workflows', $this->getInheritedWorkflows($node));
		}
		
		$pane->append('tabs', array(
			'uri' => tpl_view('issue','project',$node->nodeId,'admin','workflows'),
			'text' => SyndLib::translate('Workflows'),
			'template' => $tab,
			'selected' => $selected));
	}
	
	function _callback_workflow_event_protos(&$result) {
		$result[] = new synd_event_create(SyndNodeLib::getInstance('null.null'));
		$result[] = new synd_event_update(SyndNodeLib::getInstance('null.null'));
		$result[] = new synd_event_delete(SyndNodeLib::getInstance('null.null'));
		$result[] = new synd_event_view(SyndNodeLib::getInstance('null.null'));
		$result[] = new synd_event_mail(SyndNodeLib::getInstance('null.null'));
		$result[] = new synd_event_mail_note(SyndNodeLib::getInstance('null.null'));
	}
	
	function _callback_workflow_activity_protos(&$result, $workflow) {
		global $synd_user;
		
		// Status equals condition
		$result[] = new synd_workflow_ifstatus('synd_node_issue');

		// Create issue from template
		$result[] = new synd_workflow_clone('synd_node_issue');

		// Set project
		$result[] = new synd_workflow_project('synd_node_issue');

		// Assigned to
		$result[] = new synd_workflow_assigned('synd_node_issue');

		// Set priority
		$result[] = new synd_workflow_priority('synd_node_issue');

		// Set status
		$result[] = new synd_workflow_status('synd_node_issue');

		// Add category
		$result[] = new synd_workflow_keyword('synd_node_issue');

		// Modify resolve by
		$result[] = new synd_workflow_resolveby('synd_node_issue', '+7 days');

		// Add comment
		$result[] = new synd_workflow_comment('synd_node_issue');

		// Email to user
		$result[] = new synd_workflow_mail('synd_node_issue');

		// Save issue
		$result[] = new synd_workflow_save('synd_node_issue');

		// Edit issue
		$result[] = new synd_workflow_edit('synd_node_issue');

		// View issue
		$result[] = new synd_workflow_view('synd_node_issue');

		// Go to URL
		$result[] = new synd_workflow_href();
	}
	
	function _callback_issue_block(&$result, $block, $menu) {
		$menu->append(tpl_link('issue','workflow'), SyndLib::translate('Workflows'), '', array(new Delegate(array($this,'_callback_menu_default'),array($block)), 'run'));
	}
	
	function _callback_menu_default($block, $menu) {
		global $synd_user;
		$decider = new MethodDecider('hasPermission', array($synd_user, 'manage'));
		$projects = Module::getInstance('issue')->getProjectTree($decider, $synd_user);
		
		$query = synd_node_workflow::getEntityQuery(SyndNodeLib::getDefaultStorage('workflow'));
		$query->in('parent_node_id', new MemberIterator($projects, 'nodeId'));
		$query->where('flag_sidebar_menu', 1);
		
		foreach ($query->getEntities() as $workflow)
			$menu->append(null, $block->fetchnode($workflow, 'menuitem.tpl'));
	}
	
	function _callback_issue_context(&$result, Template $page) {
		$result .= $page->fetch('plugin/workflow/context.tpl');
	}
	
	function _callback_issue_context_menu(&$result, synd_node_issue $issue, $menu) {
		foreach (SyndLib::sort(iterator_to_array($this->getInheritedWorkflows($issue->getParent(), 1))) as $workflow)
			$menu[0]['workflows'][] = synd_node_issue::newContextOption('workflow', $workflow->id(), $workflow->toString(), null, null, false);
		if (empty($menu[0]['workflows']))
			$menu[0]['workflows'] = null;
	}
	
	function _callback_issue_context_apply(&$result, $issues, $request) {
		if (isset($request['workflow']) && null != ($workflow = SyndLib::getInstance($request['workflow']))) {
			$response = new Response();
			$workflow->process($issues, $response);
			$result = $response->toArray();
		}
	}

	function getInheritedWorkflows($node, $context = null, $sidebar = null) {
		$query = synd_node_workflow::getEntityQuery(SyndNodeLib::getDefaultStorage('workflow'));
		$query->in('parent_node_id', $node->nodeId);
		
		for ($parent = $node; !$parent->isNull(); $parent = $parent->getParent()) {
			if ($parent->isPermitted('manage'))
				$query->in('parent_node_id', $parent->nodeId);
		}
		
		$query->where('flag_prototype', 1);
		if (null !== $context)
			$query->where('flag_context_menu', $context);
		if (null !== $sidebar)
			$query->where('flag_sidebar_menu', $sidebar);
		return $query->getEntities();
	}
	
	protected function getRequestedNode($request) {
		if (!isset($request[0]))
			throw new InvalidArgumentException();
		if (null === ($node = SyndNodeLib::getInstance($request[0])))
			throw new NotFoundException();
		if (!$node->isPermitted('admin'))
			throw new ForbiddenException();
		return $node;
	}
	
	function _view_index(Request $request, Template $page, $pane = null, $title = 'Projects') {
		$module = Module::getInstance('issue');
		if (!$module->isPermitted('read'))
			throw new ForbiddenException();
		$page->assign('request', $request);
		$page->assign('plugin', $this);
		$page->assign('pane', $pane);
		$this->_title = SyndLib::translate($title);
		return array('content' => $page->fetch(tpl_design_path('plugin/workflow/index.tpl')));
	}

	function _view_newWorkflow(Request $request, Template $page) {
		$node = $this->getRequestedNode($request);
		$workflow = $node->appendChild($node->_storage->factory('workflow'));
		return array('redirect' => $page->jumpuri('issue', 'edit', $workflow->nodeId));
	}
}
