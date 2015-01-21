<?php
require_once 'core/lib/rpc/RpcChannel.class.inc';
require_once 'core/lib/rpc/RpcMessage.class.inc';
require_once 'core/lib/HttpConnection.class.inc';

/**
 * HTTP over TCP channel
 * @abstract
 * @access		public
 * @package		synd.core.lib
 */
class synd_channel_http implements RpcChannel {
	/**
	 * @access	protected
	 */
	var $_connection = null;
	
	/**
	 * @access	protected
	 */
	var $_uri = null;

	function __construct($uri) {
		$this->_connection = new HttpConnection($uri);
		$this->_uri = $uri;
	}

	function send($message) {
		$type = null != $message->getHeader('Content-Type') ? 
			$message->getHeader('Content-Type') : 'application/octet-stream';
		if (false === ($response = $this->_connection->postRequest($this->_uri, $message->toString(), $type)))
			trigger_error("Connection to remote endpoint failed", E_USER_WARNING);
		return new HttpMessage($response, $this->_connection->getHeaders());
	}
}
