<?php
require_once 'core/lib/rpc/RpcFormatter.class.inc';
require_once 'core/lib/rpc/RpcTransport.class.inc';
require_once 'core/Instance.class.inc';

/**
 * Synd instance remoting formatter
 *
 * Allows synd instances to be passed across system boundaries by 
 * encoding them as a transparent proxies.
 *
 * @access		public
 * @package		synd.core.lib
 */
class RemotingFormatter implements RpcFormatter {
	/**
	 * @access	protected
	 */
	var $_next = null;

	/**
	 * @access	protected
	 */
	var $_activator = null;
	
	/**
	 * @param	RpcFormatter	Next formatter in chain
	 * @param	RpcActivator	Server for remote object activation, set to null to disable outbound remoting (client only)
	 */
	function __construct($next, $activator) {
		$this->_next = $next;
		$this->_activator = $activator;
	}
	
	static function factory($uri) {
		return null;
	}

	function encode($method, $params = null) {
		return $this->_next->encode($method, $this->_encode($params));
	}

	function decode($message, &$method) {
		return $this->_decode($this->_next->decode($message, $method));
	}
	
	function getMessage($value) {
		return $this->_next->getMessage($this->_encode($value));
	}

	function getNativeValue($message) {
		return $this->_decode($this->_next->getNativeValue($message));
	}
	
	function getException($message, $code) {
		return $this->_next->getException($message, $code);
	}

	/**
	 * @access	protected
	 */
	function _encode($value) {
		if (is_array($value)) {
			$result = array();
			foreach (array_keys($value) as $key)
				$result[$key] = $this->_encode($value[$key]);
			return $result;
		}
		
		if (is_object($value)) {
			if (($value instanceof TransparentProxy))
				return '<synd:instance>'.$value->__transport->getEndpoint().'</synd:instance>';
			if (null !== $this->_activator && ($value instanceof Instance))
				return '<synd:instance>'.$this->_activator->getEndpoint($value).'</synd:instance>';
		}
		
		return $value;
	}
	
	/**
	 * @access	protected
	 */
	function _decode($value) {
		if (is_array($value)) {
			$result = array();
			foreach (array_keys($value) as $key)
				$result[$key] = $this->_decode($value[$key]);
			return $result;
		}
		
		if (is_string($value) && preg_match('/^<synd:instance>([^<]+)<\/synd:instance>$/S', $value, $matches))
			return Activator::getInstance($matches[1], true, $this->_activator);

		return $value;
	}
}
