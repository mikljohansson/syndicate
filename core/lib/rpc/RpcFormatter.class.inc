<?php

/**
 * Transport encodes RPC messages
 * @access		public
 * @package		synd.core.lib
 */
interface RpcFormatter {
	/**
	 * Creates a formatter for the given uri
	 * @return	RpcFormatter
	 */
	static function factory($uri);
	
	/**
	 * Encode a method invocation
	 * @param	string		Method name
	 * @param	array		Parameters to method
	 * @return	RpcMessage
	 */
	function encode($method, $params = null);

	/**
	 * Decodes a method invocation
	 * @param	RpcMessage	Encoded RPC message
	 * @param	string		Variable to store method name to
	 * @return	array		Returns the parameters, false on error
	 */
	function decode($message, &$method);
	
	/**
	 * Transport encodes a native value
	 * @param	mixed		Value to encode
	 * @return	RpcMessage
	 */
	function getMessage($value);

	/**
	 * Decodes a transport encoded message into a native value
	 * @param	RpcMessage	Message to decode
	 * @return	mixed
	 */
	function getNativeValue($message);
	
	/**
	 * Transport encodes an exception
	 * @param	string		Human readable error message
	 * @param	integer		HTTP error code
	 * @return	RpcMessage
	 */
	function getException($message, $code);
}

/**
 * @package		synd.core.lib
 */
class RemotingException extends RuntimeException {}
