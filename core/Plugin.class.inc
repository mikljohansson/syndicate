<?php
require_once 'core/Controller.class.inc';

/**
 * Plugin base class and factory
 * @access		public
 * @package		synd.core.module
 */
class Plugin extends AbstractController {
	protected $_config = null;
	
	function __construct($config = null) {
		$this->_config = $config;
	}
	
	/**
	 * Load a plugin
	 * @param	string	Name of plugin
	 * @param	array	Plugin configuration
	 * @return	Plugin	Returns a null instance on failure
	 */
	static function loadPlugin($name, $config = null) {
		$class = "synd_plugin_$name";
		return new $class($config);
	}
	
	/**
	 * Null object pattern
	 * @return	bool
	 */
	function isNull() {
		return false;
	}
}

/**
 * Null object pattern
 */
class SyndNullPlugin extends Plugin {
	function isNull() {
		return true;
	}
}
