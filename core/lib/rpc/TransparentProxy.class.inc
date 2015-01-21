<?php

/**
 * Transparent remoting proxy
 * @access		public
 * @package		synd.core.model
 */
class TransparentProxy {
	/**
	 * @var		RpcTransport
	 * @access	protected
	 */
	var $__transport = null;
	
	function TransparentProxy($transport) {
		$this->__transport = $transport;
	}

	function __call($method, $args) {
		return $this->__transport->call($method, $args);
	}
	
	function __set($key, $value) {
		$this->__transport->call('__set', array($key, $value));
	}
	
	function __get($key) {
		return $this->__transport->call('__get', array($key));
	}
}
