<?php
require_once 'core/Instance.class.inc';

/**
 * Allows for object activation
 * @access		public
 * @package		synd.core.lib
 */
interface RpcActivator {
	/**
	 * Returns the URN of an instance on this server
	 *
	 * For example
	 *  xmlrpc;http://www.example.com/synd/rpc/soap/node.issue.123/
	 *
	 * @param	Instance	Instance to return endpoint for
	 * @return	string
	 */
	function getEndpoint(Instance $instance);
}
