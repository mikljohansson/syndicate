<?php
require_once 'core/lib/rpc/RpcFormatter.class.inc';
require_once 'core/lib/rpc/RpcMessage.class.inc';

/**
 * SOAP formatter
 * @abstract
 * @access		public
 * @package		synd.core.lib
 */
abstract class synd_formatter_soap implements RpcFormatter {
	/**
	 * @access	protected
	 */
	var $_action = null;
	
	static function factory($uri) {
		require_once 'core/lib/rpc/formatter/xmlrpc.class.inc';
		$options = array(
		//	'verbosity' => 'no_white_space',
			'escaping' => array('non-ascii', 'non-print', 'markup'),
			'version' => 'soap 1.1');
		$result = new synd_formatter_soap_epi(new synd_formatter_xmlrpc_epi($options));
		$parsed = parse_url($uri);
		if (isset($parsed['host'], $parsed['path']))
			$result->setMicrosoftAction($parsed['host'], trim($parsed['path'],'/'));
		return $result;
	}
	
	/**
	 * Set a Microsoft style SOAPAction
	 *
	 * This is required when used with .NET remoting. Given an 
	 * interface named MyInterface residing in the MyNamespace 
	 * namespace in an assembly named MyProject invoke like:
	 * 
	 * setMicrosoftAction('MyProject', 'MyNamespace.MyInterface')
	 *
	 * The name of the assembly is most likely the same as your 
	 * project and resulting .exe file. Inner classes/interfaces
	 * are coded like 'SomeNamespace.MyClass+MyInterface'.
	 *
	 * @param	string	Name of assembly 
	 * @param	string	Name of the exposed interface
	 */
	function setMicrosoftAction($assembly, $interface) {
		$this->_action = "http://schemas.microsoft.com/clr/nsassem/$interface/$assembly#{method}";
	}
}

class synd_formatter_soap_epi extends synd_formatter_soap {
	/**
	 * @access	protected
	 */
	var $_next = null;

	function __construct($next) {
		$this->_next = $next;
	}
	
	function encode($method, $params = null) {
		$result = $this->_next->encode($method, $params);
		if (null != $this->_action)
			$result->setHeader('SOAPAction', str_replace('{method}', $method, $this->_action));
		return $result;
	}	

	function decode($message, &$method) {
		return $this->_next->decode($message, $method);
	}
	
	function getMessage($value) {
		return $this->_next->getMessage($value);
	}	

	function getNativeValue($message) {
		return $this->_next->getNativeValue($message);
	}
	
	function getException($message, $code) {
		return $this->_next->getException($message, $code);
	}
}
