<?php

/**
 * Encapsulates a transport encoded RPC messages
 * @access		public
 * @package		synd.core.lib
 */
interface RpcMessage {
	/**
	 * Returns a string version of this message
	 * @return	string
	 */
	function toString();
	
	/**
	 * Specify a message header
	 * @param	string	Name of header
	 * @param	string	Header value
	 */
	function setHeader($key, $value);

	/**
	 * Retrive the value of a header
	 * @param	string	Name of header
	 * @return	string	Return null if header not found
	 */
	function getHeader($key);

	/**
	 * Retrive the values of all headers
	 * @return	array
	 */
	function getHeaders();
}

/**
 * RPC message implementation
 */
class HttpMessage implements RpcMessage {
	/**
	 * @access	protected
	 */
	var $_contents = null;
	
	/**
	 * @access	protected
	 */
	var $_headers = null;
	
	function __construct($contents, $headers = null) {
		$this->_contents = $contents;
		$this->_headers = $headers;
	}
	
	function toString() {
		return $this->_contents;
	}

	function setHeader($key, $value) {
		$this->_headers[$key] = $value;
	}
	
	function getHeader($key) {
		return isset($this->_headers[$key]) ? $this->_headers[$key] : null;
	}

	function getHeaders() {
		return (array)$this->_headers;
	}
}
