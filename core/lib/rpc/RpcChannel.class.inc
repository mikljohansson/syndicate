<?php

/**
 * Transports RPC messages between endpoints
 * @access		public
 * @package		synd.core.lib
 */
interface RpcChannel {
	/**
	 * Send an encoded RPC message
	 * @param	RpcMessage	Encoded outbound message
	 * @return	RpcMessage	Returns the response message
	 */
	function send($message);
}
