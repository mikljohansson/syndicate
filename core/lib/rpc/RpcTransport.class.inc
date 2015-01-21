<?php
require_once 'core/lib/SyndLib.class.inc';
require_once 'core/lib/rpc/RemotingFormatter.class.inc';
require_once 'core/lib/rpc/TransparentProxy.class.inc';

/**
 * Handles the formatting and sending of RPC messages
 * @access		public
 * @package		synd.core.lib
 */
interface RpcTransport {
	/**
	 * Returns the endpoint of this transport
	 * @return	string
	 */
	function getEndpoint();

	/**
	 * Send an RPC message
	 * @param	string	Method name
	 * @param	array	Arguments to method
	 * @return	mixed
	 */
	function call($method, $args = array());
}

/**
 * RPC transport implementation
 * @access		public
 * @package		synd.core.lib
 */
class Transport implements RpcTransport {
	/**
	 * @access	protected
	 */
	var $_formatter = null;
	
	/**
	 * @access	protected
	 */
	var $_channel = null;
	
	/**
	 * @access	protected
	 */
	var $_endpoint = null;

	/**
	 * @param	RpcFormatter	Formatter for encoding messages
	 * @param	RpcChannel		Channel to transport messages
	 * @param	string			Endpoint of this transport
	 */
	function __construct($formatter, $channel, $endpoint) {
		$this->_formatter = $formatter;
		$this->_channel = $channel;
		$this->_endpoint = $endpoint;
	}

	/**
	 * Returns the selected endpoint
	 * @return	string
	 */
	function getEndpoint() {
		return $this->_endpoint;
	}
	
	/**
	 * Returns the current formatter
	 * @return	RpcFormatter
	 */
	function getFormatter() {
		return $this->_formatter;
	}

	function call($method, $args = array()) {
		return $this->_formatter->getNativeValue($this->_channel->send($this->_formatter->encode($method, $args)));
	}
}

/**
 * Handles remote object activation
 * @access		public
 * @package		synd.core.lib
 */
abstract class Activator {
	/**
	 * Returns a transport given an URN
	 * @param	string				Endpoint URN, for example "xmlrpc;http://www.example.com/synd/xmlrpc/node.issue.123/"
	 * @param	bool				Use object remoting for this connection
	 * @param	RpcActivator		Optional remoting activator to use, if boolean true then use the 'activator' hook
	 * @return	RpcTransport
	 */
	static function getTransport($urn, $remoting = true, $activator = null) {
		list ($formaturi, $endpointuri) = @explode(';', $urn, 2);
		list ($format) = @explode(':', $formaturi, 2);
		list ($proto) = @explode(':', $endpointuri, 2);
		
		if (!(include_once("core/lib/rpc/formatter/$format.class.inc"))) {
			trigger_error("Formatter '$format' not supported", E_USER_WARNING);
			$result = null;
			return $result;
		}

		$class = "synd_formatter_$format";
		$formatter = call_user_func(array($class, 'factory'), $formaturi);

		if (!(include_once("core/lib/rpc/channel/$proto.class.inc"))) {
			trigger_error("Channel '$proto' not supported", E_USER_WARNING);
			$result = null;
			return $result;
		}

		$class = "synd_channel_$proto";
		$channel = new $class($endpointuri);

		if ($remoting) {
			require_once 'core/lib/rpc/RemotingFormatter.class.inc';
			if (true === $activator && null === ($activator = SyndLib::runHook('activator', $format, $proto)))
				trigger_error("No remote activation available for '$format' over '$proto', ensure that the corresponding RPC module is loaded", E_USER_WARNING);
			return new Transport(new RemotingFormatter($formatter, $activator), $channel, $urn);
		}

		return new Transport($formatter, $channel, $urn);
	}
	
	/**
	 * Returns a transparent proxy given an URN
	 * @param	string				Endpoint URN, for example "xmlrpc;http://www.example.com/synd/xmlrpc/node.issue.123/"
	 * @param	bool				Use object remoting for this connection
	 * @param	RpcActivator		Optional remoting activator to use, if boolean true then use the 'activator' hook
	 * @return	TransparentProxy
	 */
	static function getInstance($urn, $remoting = true, $activator = null) {
		require_once 'core/lib/rpc/TransparentProxy.class.inc';

		list ($formaturi, $endpointuri) = @explode(';', $urn, 2);
		list ($format) = @explode(':', $formaturi, 2);
		
		switch ($format) {
			case 'local':
				return SyndLib::getInstance($endpointuri);
		}

		if (null !== ($transport = self::getTransport($urn, $remoting, $activator)))
			return new TransparentProxy($transport);

		$result = null;
		return $result;
	}
}
