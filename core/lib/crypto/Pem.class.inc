<?php

/**
 * PEM key format parser
 * @access		public
 * @package		synd.core.lib
 */
class Pem {
	const BEGIN		= 0;
	const HEADERS	= 1;
	const CONTENT	= 2;
	
	private $_type = null;
	private $_headers = array();
	private $_content = null;

	function __construct($content = null, $type = 'PGP PUBLIC KEY') {
		$this->_type = $type;
		$this->_content = null != $content ? wordwrap(base64_encode($content), 64, "\r\n", true) : null;
	}
	
	function toString() {
		$buffer = "-----BEGIN {$this->_type}-----\r\n";
		foreach ($this->_headers as $key => $value)
			$buffer .= "$key:$value\r\n";
		$buffer .= "\r\n";
		$buffer .= $this->_content;
		$buffer .= "\r\n";
		$buffer .= "-----END {$this->_type}-----";
		return $buffer;
	}
	
	function getContent() {
		return $this->_content;
	}
	
	function setHeader($key, $value) {
		$this->_headers[$key] = $value;
	}
	
	static function parse($buffer) {
		$pem = new self();
		$result = array($pem);
		
		for ($i=0, $stage = self::BEGIN, $len = strlen($buffer), $m = null; false !== $i && $i < $len; $i = strpos($buffer, "\n", $i) + 1) {
			switch ($stage) {
				case self::BEGIN:
					if ($i === stripos($buffer, '-----BEGIN ', $i)) {
						$j = $i + strlen('-----BEGIN ');
						if (false !== ($k = strpos($buffer, "-----", $j))) {
							do {$k--;} while ($k && ctype_space($buffer[$k]));
							$pem->_type = substr($buffer, $j, $k - $j + 1);
						}
						$stage = self::HEADERS;
					}
					break;
				
				case self::HEADERS:
					if ("\r" == $buffer[$i] || "\n" == $buffer[$i])
						$stage = self::CONTENT;
					else {
						if (false !== ($j = $k = strpos($buffer, ':', $i)) && false !== ($l = strpos($buffer, "\n", $k))) {
							do {$k++;} while ($k < $len && ctype_space($buffer[$k]));
							do {$l--;} while ($l && ctype_space($buffer[$l]));
							$pem->setHeader(substr($buffer, $i, $j - $i), substr($buffer, $k, $l - $k + 1));
						}
					}
					break;
				
				case self::CONTENT:
					if (null === $m)
						$m = $i;
					if ($i === stripos($buffer, '-----END ', $i)) {
						do {$i--;} while ($i && ctype_space($buffer[$i]));
						$pem->_content = substr($buffer, $m, $i - $m + 1);
						$result[] = $pem = new self();
						$stage = self::BEGIN;
					}
					break;
			}
		}
		
		return $result;
	}
}
