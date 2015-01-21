<?php
require_once 'core/lib/rpc/RpcFormatter.class.inc';
require_once 'core/lib/rpc/RpcMessage.class.inc';

/**
 * Native PHP serialization
 *
 * unserialize() might introduce security breeches relating to the
 * __wakeup() method being implicitly called on objects contained 
 * in the data received from the (untrusted) client.
 *
 * In order to subvert attempts to send objects; data is checked for
 * the existance of serialized objects before being unserialized.
 *
 * @see			serialize()
 * @access		public
 * @package		synd.core.lib
 */
class synd_formatter_php implements RpcFormatter {
	static function factory($uri) {
		return new self();
	}

	function encode($method, $params = null) {
		return $this->_message(serialize(array($method, $params)));
	}

	function decode($message, &$method) {
		if (false !== ($request = $this->getNativeValue($message))) {
			if (isset($request[0], $request[1])) {
				$method = $request[0];
				return $request[1];
			}
			
			trigger_error("Malformed request, missing index {0,1}", E_USER_WARNING);
		}
		return false;
	}
	
	function getMessage($value) {
		return $this->_message(serialize($value));
	}

	function getNativeValue($message) {
		if ($this->isSerializedObject($message->toString()))
			throw new RemotingException('Tried to unserialize object');
		if ((($result = unserialize($message->toString())) instanceof PhpFormatterException))
			throw new RemotingException($result->getMessage(), $result->getCode());
		return $result;
	}

	function getException($message, $code) {
		return $this->_message(serialize(new PhpFormatterException($message, $code)));
	}

	/**
	 * Checks for serialized object in a RPC message
	 * @param	string		Serialized string to check
	 * @access	protected
	 */
	function isSerializedObject($m) {
		for ($i=0, $j=strpos($m,'s:'), $c=strlen($m); $i<$c; $i=$j, $j=strpos($m,'s:',min($j+2,$c))) {
			if (false === $j)
				$j = $c;
			if ($i < $j && false !== ($p = strpos($m,'O:',$i)) && $p < $j && 'o:21:"phpformatterexception"' != strtolower(substr($m,$p,28)))
				return true;
			if (($j += 2) < $c) {
				$l = strpos($m,':',$j);
				$j += ($l-$j) + abs(substr($m,$j,$l-$j)) + 4;
			}
		}
		return false;
	}

	/**
	 * @access	protected
	 */
	function _message($contents) {
		return new HttpMessage($contents, array('Content-Type' => 'application/octet-stream'));
	}
}

class PhpFormatterException {
	protected $_message;
	protected $_code;
	
	function __construct($message = null, $code = 0) {
		$this->_message = $message;
		$this->_code = $code;
	}
	
	function getMessage() {
		return $this->_message;
	}
	
	function getCode() {
		return $this->_code;
	}
}
