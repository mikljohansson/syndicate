<?php
require_once 'core/lib/rpc/RpcFormatter.class.inc';
require_once 'core/lib/rpc/RpcMessage.class.inc';
require_once 'core/lib/Mime.class.inc';

/**
 * JSON serialization
 * @link		http://www.php.net/manual/en/ref.json.php
 * @access		public
 * @package		synd.core.lib
 */
class synd_formatter_json implements RpcFormatter {
	static function factory($uri) {
		return new self();
	}
	
	function encode($method, $params = null) {
		return $this->_message(json_encode(array(array($method, Mime::charset($params, null, 'utf-8')))));
	}

	function decode($message, &$method) {
		if (false !== ($request = $this->getNativeValue($message)) && isset($request[0], $request[1])) {
			$method = $request[0];
			return $request[1];
		}
		return false;
	}
	
	function getMessage($value) {
		return $this->_message(json_encode(array(Mime::charset($value, null, 'utf-8'))));
	}

	function getNativeValue($message) {
		if (null != ($value = json_decode($message->toString(), true)))
			return Mime::charset(reset($value), 'utf-8');
		return null;
	}

	function getException($message, $code) {
		return $this->getMessage(array(array('faultCode' => $code, 'faultString' => $message)));
	}

	/**
	 * @access	protected
	 */
	function _message($contents) {
		return new HttpMessage($contents, array('Content-Type' => 'text/javascript; charset="utf-8"'));
	}
}
