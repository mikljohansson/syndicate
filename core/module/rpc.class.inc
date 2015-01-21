<?php
require_once 'core/Module.class.inc';
require_once 'core/lib/rpc/RpcActivator.class.inc';

/**
 * RPC (Remote Procedure Call) base module
 *
 * Allows object in the framework to expose interfaces to RPC
 * clients using the transport implementations.
 *
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_rpc extends Module {
	private $_request = null;
	
	function __construct($config = null) {
		parent::__construct($config);
		SyndLib::attachHook('activator', array($this, '_callback_activator'));
	}

	function request(Request $request) {
		global $HTTP_RAW_POST_DATA;
		if (!isset($request[0]) || !preg_match('/^\w+$/', $request[0]) || !class_exists($class = 'synd_formatter_'.$request[0]))
			throw new InvalidArgumentException();
		
		require_once 'core/lib/rpc/RemotingFormatter.class.inc';
		$formatter = new RemotingFormatter(
			call_user_func(array($class, 'factory'), $request[0]), 
			new RpcModuleActivator($this, $request[0]));
		$message = new HttpMessage($HTTP_RAW_POST_DATA, 
			function_exists('apache_request_headers') ? apache_request_headers() : null);
		$this->_request = $request;
		
		try {
			if (false === ($args = $formatter->decode($message, $method)))
				throw new InvalidArgumentException('Failed to decode request');
			$response = $formatter->getMessage($this->_request(
				isset($request[1]) ? $request[1] : null, $method, $args, 
				isset($request[2]) ? $request[2] : null));
		}
		catch (InvalidArgumentException $e)	{$response = $formatter->getException($e->getMessage(), 400);}
		catch (ForbiddenException $e)		{$response = $formatter->getException($e->getMessage(), 403);}
		catch (NotFoundException $e)		{$response = $formatter->getException($e->getMessage(), 404);}
		catch (RuntimeException $e)			{$response = $formatter->getException($e->getMessage(), 500);}
		
		// Send response headers
		if (null == $response->getHeader('Content-Type'))
			$response->setHeader('Content-Type', 'application/octet-stream');
		foreach ($response->getHeaders() as $key => $value)
			header("$key: $value");
		
		return array('content' => $response->toString(), 'page' => null);
	}

	protected function _request($id, $method, $args, $token = null) {
		if (null == $id)
			$instance = $this;
		else if (null == ($instance = SyndLib::getInstance($id)))
			throw new NotFoundException("Endpoint $id not found");

		if (method_exists($instance, "_public_$method"))
			return call_user_func_array(array($instance, "_public_$method"), (array)$args);

		if (null != $token) {
			if (!$this->isValidToken($id, $token))
				throw new ForbiddenException('Invalid or expired autentication token');
		}
		else {
			if (!method_exists($instance, 'isPermitted') || !$instance->isPermitted('invoke', $method))
				throw new ForbiddenException("Method '$method' not accessible on instance '$id'");
		}
		
		if (!method_exists($instance, $method))
			throw new InvalidArgumentException('Method not found');
		return call_user_func_array(array($instance, $method), (array)$args);
	}
	
	/**
	 * Implements the 'activator' hook
	 *
	 * This method must be overridden by any child classes
	 *
	 * @param	string	The desired formatter to support {'php','xmlrpc','soap','json'}
	 * @param	string	The desired channel transport method {'http','https'}
	 */
	function _callback_activator(&$result, $formatter, $channel) {
		if ('http' == $channel || 'https' == $channel) {
			$result = new RpcModuleActivator($this, $formatter);
			return true;
		}
	}

	/**
	 * Creates an authentication token
	 * @access	protected
	 * @param	string		Identifier to authenticate
	 * @param	integer		Time to live in seconds
	 * @return	string
	 */
	function getToken($id, $ttl = 86400) {
		if (null == ($key = SyndLib::runHook('variable_get', 'rpc', 'private_key')))
			SyndLib::runHook('variable_set', 'rpc', 'private_key', $key = md5(uniqid(rand(), true)));
		$expire = dechex(time() + $ttl);
		return md5("$id:$expire").$expire;
	}
	
	/**
	 * Validates an authentication token
	 * @access	protected
	 * @param	string		Identifier to authenticate
	 * @param	string		Token to validate
	 * @return	bool
	 */
	function isValidToken($id, $token) {
		if (40 == strlen($token)) {
			$key = SyndLib::runHook('variable_get', 'rpc', 'private_key');
			$hash = substr($token,0,32);
			$expire = substr($token,32,8);
			if (md5("$id:$expire") == $hash && hexdec($expire) >= time())
				return true;
		}		
		return false;
	}
	
	/**
	 * Executes multiple RPC calls
	 *
	 * <code>
	 * array(
	 *   array('issue.123', 'setAssigned', array('user.1')),
	 *   array('project.1', 'getContextMenu', array('project.2')),
	 * )
	 * </code>
	 *
	 * @param	array	List of call specs
	 */
	function _public_multicall($callspecs) {
		$result = array();
		foreach ($callspecs as $spec)
			$result[] = $this->_request($spec[0], $spec[1], $spec[2], isset($spec[3]) ? $spec[3] : null);
		return $result;
	}

	/**
	 * Exposes the request chain to RPC
	 * @see		RequestProcessor::request()
	 * @param	string	Requested context path
	 * @param	array	Request query parameters
	 * @return	string
	 */
	function _public_request($context, $query) {
		$variables = array();
		parse_str(parse_url($context, PHP_URL_QUERY), $variables);

		if (is_string($query)) {
			$parsed = array();
			parse_str($query, $parsed);
			$variables = array_merge($variables, $parsed);
		}
		else if (is_array($query)) {
			$variables = array_merge($variables, $query);
		}

		$request = new HttpRequest(
			$_SESSION, $this->_request->uribase, $context, 
			$variables, $variables, array(), array(), $_COOKIE);
		$processor = new RequestProcessor();
		$response = $processor->request($request);
		
		return isset($response['content']) ? $response['content'] : false;
	}
}

class RpcModuleActivator implements RpcActivator {
	private $_module;
	private $_formatter;
	
	function __construct($module, $formatter) {
		$this->_module = $module;
		$this->_formatter = $formatter;
	}
	
	function getEndpoint(Instance $instance) {
		return $this->_formatter.';'.tpl_request_host().tpl_view('rpc', $this->_formatter, $instance->id(), $this->_module->getToken($instance->id()));
	}
}
