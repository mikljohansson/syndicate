<?php
require_once 'core/Module.class.inc';
require_once 'core/lib/Template.class.inc';

/**
 * Node handling module
 *
 * Modules can inherit from this to provide node integration and 
 * provide extended functionality such as sidebars and OLE.
 *
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_node extends Module {
	/**
	 * The active node, set by _view_view() etc.
	 */
	protected $_node = null;
	
	/**
	 * Page template
	 */
	private $_template = null;
	
	function __construct($config = null) {
		parent::__construct($config);
		if (get_class($this) == __CLASS__)
			SyndLib::attachHook('search', array($this, '_callback_search'), -100);
	}
	
	function request(Request $request) {
		SyndLib::attachHook('html_head_title', array($this, '_callback_html_head_title'), -100);
		return parent::request($request);
	}
	
	/**
	 * @access	protected
	 */
	function _callback_html_head_title(&$result) {
		if (null !== ($node = $this->getActiveNode()) && $node->isPermitted('read'))
			return $result = $node->toString();
	}

	function getActiveNode() {
		if (null != $this->_node)
			return $this->_node;
		return SyndNodeLib::getInstance('null.null');
	}
	
	function getTemplate() {
		global $synd_config;
		if (null == $this->_template)
			$this->_template = new Template(array_reverse($synd_config['dirs']['design']));
		return $this->_template;
	}
	
	/**
	 * Invokes a view on a node.
	 *
	 * Method should have the prototype "mres = _view_{$method}(array $request)"
	 * and will have to do its own access control and data validation
	 */
	function _view_invoke(Request $request, Template $page) {
		if (!isset($request[0], $request[1]))
			throw new NotFoundException();
		
		if (false !== strpos($request[0], '.')) {
			if (null === ($this->_node = SyndNodeLib::getInstance($request[0])))
				throw new NotFoundException();
			$function = array($this->_node, '_view_'.str_replace('.','_',$request[1]));
		}
		else {
			if (null == ($class = SyndNodeLib::loadClass($request[0])))
				throw new NotFoundException();
			$function = array($class, '_view_'.str_replace('.','_',$request[1]));
		}

		if (!is_callable($function)) {
			// Support deprecated '_public_' naming of node views
			$function[1] = '_public_'.str_replace('.','_',$request[1]);
			if (!is_callable($function))
				throw new NotFoundException();
		}
		return call_user_func($function, $request, $page);
	}

	/**
	 * Displays the full_view.tpl of a node
	 */
	function _view_view(Request $request, Template $page) {
		if (!isset($request[0]) || null === ($this->_node = SyndNodeLib::getInstance($request[0])))
			throw new NotFoundException();

		$forward = $request->forward();
		if (isset($forward[0]) && $this->_node->isPermitted('read', "full_view_{$forward[0]}.tpl")) {
			$view = "full_view_{$forward[0]}.tpl";
			$forward = $forward->forward();
		}
		else if ($this->_node->isPermitted('read'))
			$view = "full_view.tpl";
		else
			throw new ForbiddenException();

		$page->assign('request', $forward);
		return array('content' => $page->fetchnode($this->_node, $view));
	}

	/**
	 * Displays the full_edit.tpl of a node
	 */
	function _view_edit(Request $request, Template $page) {
		if (!isset($request[0]) || null === ($this->_node = SyndNodeLib::getInstance($request[0])))
			throw new NotFoundException();
		return $this->_node->_view_edit($request->forward(), $page);
	}

	/**
	 * @deprecated
	 */
	function _view_insert(Request $request, Template $page) {
		// Allow child modules to set the active node
		if (null == $this->_node) {
			if (isset($request['node_id']) && null === ($this->_node = SyndNodeLib::getInstance($request['node_id'])))
				throw new NotFoundException();

			// Clone new node from prototype if not already cloned from same
			if (!empty($request['clone']) && (!isset($this->_node->__cloned) || $this->_node->__cloned != $request['clone'])) {
				if (null == ($template = SyndNodeLib::getInstance($request['clone'])))
					throw new NotFoundException();
				$this->_node = $template->copy();
				$this->_node->__cloned = $template->nodeId;
			}
			else if (null === $this->_node) {
				if (!empty($request[0]))
					$this->_node = SyndNodeLib::factory($request[0]);
				else if (!empty($request['class_id']))
					$this->_node = SyndNodeLib::factory($request['class_id']);
			}
			
			if (null === $this->_node)
				throw new NotFoundException();
		}
		
		if (!$this->_node->isPermitted('write'))
			throw new ForbiddenException();

		$errors = array();
		
		// Merge submitted data if we didn't just make a new clone
		if (isset($request['data']) && !isset($template) && 
			!count($errors = $this->_node->validate($request['data'])))
			$this->_node->merge($request['data']);
			
		if (isset($request['post']) && empty($errors)) {
			$this->_node->save();
			$this->_node->flush(); // Commit node to finalize it's nodeId
			return array('redirect' => tpl_uri_return(tpl_view($this->_node->getHandler(),'view',$this->_node->nodeId())));
		}

		// Determine view to display
		$view = 'full_edit.tpl';
		if (isset($request[1]) && $this->_node->isPermitted('read',"full_edit_{$request[1]}.tpl"))
			$view = "full_edit_{$request[1]}.tpl";

		$params = array(
			'node' => $this->_node,
			'view' => $view,
			'data' => $this->_node->getCompositeData());
		if (!empty($errors))
			$params['errors'] = $errors;

		// Merge original node data and submitted data
		if (isset($request['data'])) {
			$params['data'] = isset($template) ? 
				SyndLib::array_merge_assoc($request['data'], $params['data']) : 
				SyndLib::array_merge_assoc($params['data'], $request['data']);
		}

		$page->assign('request', $request->forward());
		return array('content' => $page->fetch('module/node/insert.tpl', $params));
	}

	/**
	 * Deletes a node.
	 *
	 * Asks for confirmation an displays the line_view.tpl of a node
	 */
	function _view_delete(Request $request, Template $page) {
		if (null == ($this->_node = SyndNodeLib::getInstance($request[0])))
			throw new NotFoundException();
		if (!$this->_node->isPermitted('write'))
			throw new ForbiddenException();
		
		$mres = array();
		if (isset($request['confirm'])) {
			if (isset($request['redirect']))
				$mres['redirect'] = $request['redirect'];
			else if (null != ($parent = $this->_node->getParent()) && !$parent->isNull())
				$mres['redirect'] = tpl_uri_return(tpl_view($parent->getHandler(),'view',$parent->nodeId));
			else
				$mres['redirect'] = tpl_uri_return(tpl_view($parent->getHandler()));
			$this->_node->delete();
		}
		else {
			$mres['content'] = $page->fetch('module/node/delete.tpl', array('node' => $this->_node));
		}
		
		return $mres;
	}
	
	/**
	 * Node wizard interface
	 * @param	array
	 */
	function _view_flow(Request $request, Template $page) {
		if (false !== strpos($request[0],'.'))
			$this->_node = SyndNodeLib::getInstance($request[0]);
		else
			$this->_node = SyndNodeLib::factory($request[0]);
		
		$view = $this->_flowTemplate($request);
		$data = $request['data'];
		$errors = array();
		$isValid = true;

		if (null == $this->_node)
			throw new NotFoundException();
		if (!$this->_node->isPermitted('write') || !$this->_node->isPermitted('read', $view))
			throw new ForbiddenException();
		if ($isValid = $this->_node->validateFlow($data, $errors, $view))
			$redirect = $this->_node->mergeFlow($data, $view);
		
		if (isset($request['post']) && $isValid) {
			// Try to redirect to next step
			if (!empty($redirect) || null != ($redirect = $this->_nextLocation($request))) {
				$this->_node->_save();
				return array('redirect' => $redirect);
			}
			
			$this->_node->save();
			$this->_node->flush();
			return array('redirect' => tpl_uri_return(tpl_view($this->_node->getHandler(),'view',$this->_node->nodeId)));
		}

		$page->assign('request', $request->forward());

		// Render current step
		$params = array(
			'node' => $this->_node,
			'view' => $view,
			'data' => $this->_node->getCompositeData());
		if (false == $isValid)
			$params['errors'] = $errors;

		// Merge original node data and submitted data
		if (isset($request['data']))
			$params['data'] = SyndLib::array_merge_assoc($params['data'], $request['data']);

		return array('content' => $page->fetch('module/node/edit.tpl', $params));
	}
	
	/**
	 * @access	private
	 */
	function _flowTemplate($request) {
		if (is_numeric($request[1]))
			return "flow/step{$request[1]}.tpl";
		else if (preg_match('/^\w+$/', $request[1]) && is_numeric($request[2]))
			return "flow/{$request[1]}/step{$request[2]}.tpl";
		return null;
	}

	/**
	 * @access	private
	 */
	function _nextLocation($request) {
		$template = null;
		if (is_numeric($request[1])) {
			$template = "flow/step".($request[1]+1).".tpl";
			$path = tpl_view_jump($this->_node->getHandler(), 'flow', $this->_node->nodeId, $request[1]+1);
		}
		else if (preg_match('/^\w+$/', $request[1]) && is_numeric($request[2])) {
			$template = "flow/{$request[1]}/step".($request[2]+1).".tpl";
			$path = tpl_view_jump($this->_node->getHandler(), 'flow', $this->_node->nodeId, $request[1], $request[2]+1);
		}

		if (null != $template && null != tpl_gui_path(get_class($this->_node), $template, false))
			return $path;
		return null;
	}

	/**
	 * Node search hook
	 * @param Collection	Collection of matches
	 * @param	string			Class to find matches of
	 * @param	string			Query string
	 * @param	array			Order by fields as array(string field, boolean ascending)
	 * @param	bool			Block hook if no matches
	 * @param	string			Default operator {'&&','||'}
	 */
	function _callback_search(&$result, $clsid, $search, $order = null, $block = true, $operator = '&&') {
		$result = SyndType::factory('search_collection', $clsid, $search, $order, $operator);
		return $block || $result->getCount();
	}
	
	/**
	 * @access	private
	 */
	static function _runHook($name, &$result) {
		foreach (array_unique(explode(':', ini_get('include_path'))) as $path) {
			if (is_dir($dir = rtrim($path,'/').'/core/model/node/')) {
				foreach (SyndLib::scanDirectory($dir) as $file) {
					if (!is_file($file))
						continue;
					$class = 'synd_node_'.SyndLib::chopExtension(SyndLib::chopExtension(basename($file)));
					if (!class_exists($class, false) && !(include_once($file)))
						continue;
					if (is_callable($function = array($class,"_node_hook_$name")))
						call_user_func_array($function, array(&$result));
				}
			}
		}
	}

	/**
	 * Propagates the 'cron' hook to node classes
	 * @access	protected
	 */
	function _hook_cron() {
		synd_module_node::_runHook('cron', $result);
	}
}
