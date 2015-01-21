<?php
require_once 'core/Module.class.inc';
require_once 'core/lib/CacheStrategy.class.inc';

/**
 * File and memory cache module
 * @see			CacheStrategy
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_cache extends Module {
	/**
	 * @access	protected
	 */
	var $_strategy = null;
	
	/**
	 * Registers cache hooks.
	 */
	function __construct() {
		// Don't load cache module for shell scripts
		if (!isset($_SERVER['REMOTE_ADDR']))
			return;
		
		$this->_strategy = CacheStrategyManager::factory();

		if (isset($_SESSION['synd']['cache']['disable']))
			return;

		SyndLib::attachHook('cache_set', array($this, '_callback_cache_set'));
		SyndLib::attachHook('cache_get', array($this, '_callback_cache_get'));
		SyndLib::attachHook('cache_del', array($this, '_callback_cache_del'));

		SyndLib::attachHook('variable_set', array($this, '_callback_variable_set'), 10);
		SyndLib::attachHook('variable_get', array($this, '_callback_variable_get'), 10);

		SyndLib::attachHook('menu_admin', array($this, '_callback_menu_admin'));
	}

	/**
	 * @access	protected
	 */
	function _hook_manage_permissions(&$result) {
		$result[] = $this;	
	}

	function getDescription() {
		return SyndLib::translate('Memory and disk cache.');
	}

	function _callback_menu_admin(&$result, $menu) {
		if ($this->isPermitted('admin'))
			$menu->append(tpl_link('system','cache'), SyndLib::translate('Cache'), '', array($this, '_callback_menu_admin_index'), array('system/cache/*'));
	}
	
	function _callback_menu_admin_index($menu) {
		$menu->append(tpl_link_call('cache','clear'), SyndLib::translate('Clear cache'));
	}
	
	/**
	 * @access	private
	 */
	function _callback_design_menu_full(&$result) {
		if ($this->isPermitted('admin')) {
			require_once 'core/lib/TemplateLib.inc';
			$result['admin']['cache'][] =
				array('text' => 'Clear cache', 'uri' => tpl_view_call('cache', 'clear'));
			
			if (isset($_SESSION['synd']['cache']['disable'])) {
				$result['admin']['cache'][] = array(
					'text' => 'Enable cache', 'uri' => tpl_view_call('cache','enable'));
			}
			else {
				$result['admin']['cache'][] = array(
					'text' => 'Disable cache', 'uri' => tpl_view_call('cache','disable'));
			}
		}
	}

	/**
	 * Displays cache config and statistics.
	 */
	function _view_index(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		SyndLib::attachHook('design_menu', array($this, '_callback_design_menu_full'));
		return array('content' => $page->fetch('module/cache/index.tpl'));
	}

	function _view_enable() {
		global $synd_config;
		if (isset($_SERVER['REMOTE_ADDR'], $synd_config['debug_allowed_ips'][$_SERVER['REMOTE_ADDR']]) || 
			$this->isPermitted('read')) {
			unset($_SESSION['synd']['cache']['disable']);
			$this->_strategy->clear();
		}
		else
			throw new ForbiddenException();
	}
	
	function _view_disable() {
		global $synd_config;
		if (isset($_SERVER['REMOTE_ADDR'], $synd_config['debug_allowed_ips'][$_SERVER['REMOTE_ADDR']]) || 
			$this->isPermitted('read')) 
			$_SESSION['synd']['cache']['disable'] = true;
		else
			throw new ForbiddenException();
	}

	function _view_clear() {
		global $synd_config;
		if (isset($_SERVER['REMOTE_ADDR'], $synd_config['debug_allowed_ips'][$_SERVER['REMOTE_ADDR']]) || $this->isPermitted('read')) {
			CacheStrategyManager::factory()->clear();
			CacheStrategyManager::localized()->clear();
		}
		else
			throw new ForbiddenException();
	}
	
	/**
	 * @access	protected
	 */
	function _callback_cache_set(&$result, $key, $value, $ttl = 93600) {
		return $this->_strategy->set($key, $value, $ttl);
	}

	/**
	 * 'cache_get' implementation
	 * @access	protected
	 */
	function _callback_cache_get(&$result, $key) {
		if (false !== ($value = $this->_strategy->get($key))) {
			$result = $value;
			return true;
		}
	}

	/**
	 * 'cache_del' implementation
	 * @access	protected
	 */
	function _callback_cache_del(&$result, $key) {
		$this->_strategy->delete($key);
	}

	/**
	 * Cached 'variable_set' interceptor
	 * @access	protected
	 */
	function _callback_variable_set(&$result, $namespace, $variable, $value, $ttl = null) {
		if (null === $value)
			$this->_strategy->delete($namespace.'.'.$variable);
		else
			$this->_strategy->set($namespace.'.'.$variable, $value, $ttl);
	}

	/**
	 * Cached 'variable_get' interceptor
	 * @access	protected
	 */
	function _callback_variable_get(&$result, $namespace, $variable = null) {
		if (false !== ($value = $this->_strategy->get($namespace.'.'.$variable))) {
			$result = $value;
			return true;
		}
	}
}
