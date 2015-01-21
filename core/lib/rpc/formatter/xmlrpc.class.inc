<?php
require_once 'core/lib/rpc/RpcFormatter.class.inc';
require_once 'core/lib/rpc/RpcMessage.class.inc';

/**
 * XML-RPC formatter
 * @abstract
 * @access		public
 * @package		synd.core.lib
 */
abstract class synd_formatter_xmlrpc implements RpcFormatter {
	static function factory($uri) {
		if (function_exists('xmlrpc_encode_request'))
			return new synd_formatter_xmlrpc_epi();
		return null;
	}
	
	function getException($message, $code) {
		return $this->getMessage(array('faultCode' => $code, 'faultString' => $message));
	}
	
	/**
	 * @access	protected
	 */
	function _message($contents) {
		if (null == ($charset = ini_get('default_charset')))
			$charset = 'utf-8';
		return new HttpMessage($contents, array('Content-Type' => 'text/xml; charset="'.$charset.'"'));
	}
}

/**
 * XMLRPC-EPI implementation (./configure --with-xmlrpc)
 *
 * This implementation does not handle hashmaps with numeric keys
 * correcly but returns them zero based.
 *
 * @access	protected
 */
class synd_formatter_xmlrpc_epi extends synd_formatter_xmlrpc {
	/**
	 * @access	protected
	 */
	var $_options = null;
	
	/**
	 * @link	http://xmlrpc-epi.sourceforge.net/main.php?t=php_api#data
	 * @param	array	xmlrpc_encode_request output_options
	 */
	function __construct($options = null) {
		if (null !== $options) 
			$this->_options = $options;
		else {
			if (null == ($charset = ini_get('default_charset')))
				$charset = 'utf-8';
			$this->_options = array(
			//	'verbosity'	=> 'no_white_space',
				'escaping'	=> array('markup'),
				'version'	=> 'xmlrpc',
				'encoding'	=> $charset);
		}
	}

	function encode($method, $params = null) {
		return $this->_message(xmlrpc_encode_request($method, $this->_encode($params), $this->_options));
	}

	function decode($message, &$method) {
		return $this->_decode(xmlrpc_decode_request($message->toString(), $method));
	}
	
	function getMessage($value) {
		return $this->_message(xmlrpc_encode_request(null, $this->_encode($value), $this->_options));
	}

	function getNativeValue($message) {
		return $this->_decode(xmlrpc_decode_request($message->toString(), $method));
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
		
		if (is_object($value))
			return $this->_encode(get_object_vars($value));
		
		if (is_string($value) && preg_match('/[\x00-\x08\x0B-\x0C\x0E-\x1F]/S', $value)) {
			$clone = $value;
			xmlrpc_set_type($clone, 'base64');
			return $clone;
		}
		
		return $value;
	}

	/**
	 * @access	protected
	 */
	function _decode($value) {
		if (is_array($value)) {
			if (2 == count($value) && isset($value['faultCode'], $value['faultString'])) {
				trigger_error($this->_decode($value['faultString']).' ('.$this->_decode($value['faultCode']).')', E_USER_WARNING);
				return false;
			}
			
			foreach ($value as $key => $val)
				$value[$key] = $this->_decode($val);
			return $value;
		}
		
		if (is_object($value))
			return $value->scalar;
		if (is_null($value))
			return false;
		
		return $value;
	}
}

