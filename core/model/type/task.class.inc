<?php

class synd_type_task extends SyndType {
	/**
	 * $synd_config that will be restored on run()
	 * @access	protected
	 */
	var $_config = null;

	/**
	 * $_SERVER that will be restored on run()
	 * @access	protected
	 */
	var $_serv = null;

	/**
	 * Task callback function
	 * @access	protected
	 */
	var $_callback = null;

	/**
	 * Arguments that will be passed to callback
	 * @access	protected
	 */
	var $_args = null;
	
	function __construct($oid = null, $callback = null, $args = null) {
		parent::__construct($oid);
		global $synd_config;
		assert('null === $callback || is_callable($callback)');
		$this->_config = $synd_config;
		$this->_serv = $_SERVER;
		$this->_callback = $callback;
		$this->_args = $args;
	}
	
	function run($handler) {
		global $synd_config;
		$synd_config = $this->_config;
		foreach ($this->_serv as $key => $value)
			$_SERVER[$key] = $value;
		if (null !== $this->_callback)
			call_user_func_array($this->_callback, (array)$this->_args);
	}
}
