<?php
require_once 'core/Controller.class.inc';
require_once 'core/Instance.class.inc';
require_once 'core/Plugin.class.inc';
require_once 'core/lib/SyndLib.class.inc';

/**
 * Module base class and factory
 * @access		public
 * @package		synd.core.module
 */
abstract class Module extends AbstractController implements Instance {
	private static $_instances = array();
	protected $_config = null;
	protected $_plugins = null;
	
	/**
	 * @param	mixed	Module configuration
	 */
	function __construct($config = null) {
		$this->_config = $config;
		if (!empty($this->_config['plugins'])) {
			foreach ($this->_config['plugins'] as $name => $conf)
				$this->_plugins[$name] = Plugin::loadPlugin($name, $conf);
		}
	}
	
	/**
	 * Module instance accessor
	 * @param	string		Name of module
	 * @return	Module
	 */
	static function getInstance($id) {
		global $synd_config;
		if (isset(self::$_instances[$id])) 
			return self::$_instances[$id];
		if (class_exists($class = "synd_module_$id"))
			return self::$_instances[$id] = new $class(isset($synd_config[$id]) ? $synd_config[$id] : null);
		return null;
	}

	static function __callback_instance(&$result, $id) {
		if (isset(self::$_instances[$id])) {
			$result = self::$_instances[$id];
			return true;
		}
	}
	
	function _hook_loadInstance($id) {
		return isset($this->_plugins[$id]) ? $this->_plugins[$id] : null;
	}

	function request(Request $request) {
		if (false === ($result = parent::request($request)) && isset($request[0], $this->_plugins[$request[0]]))
			$result = $this->_plugins[$request[0]]->request($request->forward());
		return $result;
	}

	/**
	 * Loads a plugin
	 * @see		Plugin::loadPlugin()
	 * @param	string		Name of plugin
	 * @return	Plugin
	 */
	function loadPlugin($name) {
		if (!isset($this->_plugins[$name]))
			$this->_plugins[$name] = Plugin::loadPlugin($name);
		return $this->_plugins[$name];
	}

	static function runHook($hook) {
		$result = null;
		$args = func_get_args();
		$args[0] =& $result;
		
		foreach (self::$_instances as $module) {
			if (is_callable($function = array($module, '_hook_'.$hook)) && true === call_user_func_array($function, $args))
				break;
		}

		return $result;
	}

	/**
	 * Returns the classId of this module
	 * @return	string
	 */
	function classId() {
		return substr(get_class($this), strlen('synd_module_'));
	}

	/**
	 * Null object pattern
	 * @return	bool
	 */
	function isNull() {
		return false;
	}
	
	/**
	 * Returns the id of this module
	 * @return	string
	 */
	function id() {
		return substr(get_class($this), strlen('synd_module_'));
	}
	
	function toString() {
		return ucfirst($this->id());
	}
	
	function getDescription() {
		return '';
	}
	
	/**
	 * Checks if an operation is allowed on this module.
	 * @param	string	Operation to check. {read,write,admin,...}
	 * @param	mixed	Additional argument
	 * @return	bool
	 */
	function isPermitted($perm, $arg = null) {
		global $synd_user;
		return $this->hasPermission($synd_user, $perm, $arg);
	}

	/**
	 * Checks if an operation is allowed on this node.
	 * @param	User	User to check permission for
	 * @param	string	Operation to check. {read,write,...}
	 * @param	mixed	Additional argument
	 * @return	bool
	 */
	function hasPermission($user, $perm, $arg = null) {
		if ('invoke' == $perm && in_array($arg, array('instanceid','classid')))
			return true;
		return (bool)SyndLib::runHook('permission', $this, $user, $perm);
	}
	
	/**
	 * Returns the defined permissions for this node
	 * @return	array	Array of descriptions indexed by permission
	 */
	function getDefinedPermissions() {
		return array(
			'read' => SyndLib::translate('Allows basic access to this module'),
			'write' => SyndLib::translate('Allows write access to this module'),
			'admin' => SyndLib::translate('Allows module administrative access and as well as changing permissions'),
			);
	}
	
	/**
	 * Is a permission inherited by default?
	 */
	function isDefaultInherited($op) {
		return false;
	}

	/**
	 * Returns the available roles
	 *
	 * A role is a role or relationship a user can have to this node,
	 * for example the project nodes has the 'case.Registered' role
	 * for defining project members.
	 *
	 * @return	array	Array of nodes
	 */
	function getDefinedRoles() {
		return (array)Module::runHook('defined_roles', array($this));
	}
	
	function getParent() {
		return new SyndNullModule();
	}
}

/**
 * Null object pattern
 */
class SyndNullModule extends Module {
	function isNull() {
		return true;
	}
}

SyndLib::attachHook('instance', array('Module', '__callback_instance'));
