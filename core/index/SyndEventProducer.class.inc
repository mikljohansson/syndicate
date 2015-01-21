<?php

/**
 * Implements methods for attaching to and running hooks
 *
 * @abstract
 * @access		public
 * @package		synd.core.index
 */
class SyndEventProducer {
	/**
	 * Loaded extensions
	 * @access	protected
	 */
	var $_extensions = null;
	
	/**
	 * Attaches hooks
	 * @access	protected
	 */
	var $_hooks = array();

	/**
	 * Load an extension
	 * @param	SyndIndexExtension
	 */
	function loadExtension($extension) {
		$this->_extensions[] = $extension;
		$extension->initialize($this);
	}
	
	function getExtensions() {
		return $this->_extensions;
	}
	
	/**
	 * Runs an index hook
	 * @param	string	Name of hook
	 * @param	array	Arguments to callback
	 * @return	mixed
	 */
	function runHook($name, $args = null) {
		if (!isset($this->_hooks[$name]))
			return null;
		
		$args = array_merge(array(null), (array)$args);
		$args[0] =& $result;
		
		foreach (array_keys($this->_hooks[$name]) as $key) {
			if (call_user_func_array($this->_hooks[$name][$key], $args))
				break;
		}
		
		return $result;
	}
	
	/**
	 * Attaches to an index hook
	 * @uses	call_user_func_array()
	 * @param	string	Name of hook
	 * @param	mixed	Callback to run 
	 * @param	integer	Higher priority get run first
	 */
	function attachHook($name, $callback, $priority = 0) {
		assert('is_callable($callback)');
		if (!isset($this->_hooks[$name]))
			$this->_hooks[$name] = array();
		
		while (isset($this->_hooks[$name][$priority]))
			$priority--;
		
		$this->_hooks[$name][$priority] = $callback;
		krsort($this->_hooks[$name]);
	}
}
