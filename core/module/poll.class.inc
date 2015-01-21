<?php
require_once 'core/module/node.class.inc';

/**
 * Poll module
 *
 * Provides integration for users to create polls and other forms.
 *
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_poll extends synd_module_node {
	function getDescription() {
		return SyndLib::translate('Provides integration for users to create polls and other forms.');
	}
	
	/**
	 * @access	protected
	 */
	function _hook_manage_permissions(&$result) {
		$result[] = $this;	
	}
	
	/**
	 * Attach display hooks
	 */
	function request(Request $request) {
		SyndLib::attachHook('block', array($this, '_callback_block'));
		tpl_load_script(tpl_design_uri('js/ole.js'));
		return parent::request($request);
	}

	/**
	 * Resolve '/poll/123/' style uri's
	 */
	function _hook_resolve_uri(&$result, $path) {
		SyndNodeLib::loadClass('poll');
		$parts = explode('/', $path, 3);
		if (isset($parts[1]) && is_numeric($parts[1]) && 
			null != ($poll = SyndNodeLib::getInstance(synd_node_poll::resolve($parts[1])))) {
			$result = "{$parts[0]}/view/poll.{$parts[1]}/";
			if (isset($parts[2])) 
				$result .= $parts[2];
			return true;
		}
	}
	
	function _view_index(Request $request, Template $page) {
		global $synd_user;
		$storage = SyndNodeLib::getDefaultStorage('poll');
		$database = $storage->getDatabase();
		
		$sql = "
			SELECT p.node_id FROM synd_poll p
			WHERE p.create_node_id = ".$database->quote($synd_user->nodeId);
		$polls = $storage->getInstances($database->getCol($sql));

		$page->assign('myPolls', $polls);
		return array('content' => $page->fetch('module/poll/index.tpl'));
	}
	
	function _callback_block(&$result, Template $page) {
		$poll = $this->getActiveNode();
		if (!$poll->isNull()) {
			if ($poll->isPermitted('write')) {
				require_once 'design/gui/MenuBuilder.class.inc';
				$menu = new ListMenu();
				$menu->append(null, SyndLib::translate('Commands'), '', array($this, '_callback_menu_commands'));

				$result[] = array(
					'text' => SyndLib::translate('Poll'),
					'content' => $page->fetch(tpl_design_path('module/poll/blocks/default.tpl'), array('menu'=>$menu)));
			}
			
			if (!empty($_SESSION['synd']['poll']['ole'])) {
				foreach (array_keys($_SESSION['synd']['poll']['ole']) as $key) {
					if ($_SESSION['synd']['poll']['ole'][$key]->isCancelled())
						unset($_SESSION['synd']['poll']['ole'][$key]);
					else {
						$result[] = array('content' => 
							$page->fetchnode($_SESSION['synd']['poll']['ole'][$key],'full_view.tpl',
							array('target' => $poll->getPage())));
					}
				}
			}
		}
	}

	function _callback_menu_commands($menu) {
		$menu->append("javascript:synd_ole_call('".tpl_link_call('poll','cut')."',true)", SyndLib::translate('Cut'),
			SyndLib::translate('Accesskey: %s','X'), null, null, 'x');
		$menu->append("javascript:synd_ole_call('".tpl_link_call('poll','copy')."',true)", SyndLib::translate('Copy'),
			SyndLib::translate('Accesskey: %s','C'), null, null, 'c');
		$menu->append("javascript:synd_ole_call('".tpl_link_call('poll','delete')."',true)", SyndLib::translate('Delete'));
	}

	/**
	 * Creates an new poll
	 */
	function _view_new(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		$poll = SyndNodeLib::factory('poll');
		return array('redirect' => tpl_view($poll->getHandler(),'edit',$poll->nodeId));
	}
	
	function _getCollection($request) {
		if (empty($request['collections']))
			return;

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
		
		$_SESSION['synd']['poll']['ole'][] = $action;
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
		
		$_SESSION['synd']['poll']['ole'][] = $action;
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
		}
		else {
			$page->assign('contents', $contents);
			$page->assign('collection', $collection);
			return array('content' => $page->fetch('module/poll/delete.tpl'));
		}
	}
}
