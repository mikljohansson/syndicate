<?php

/**
 * HTTP/1.1 connection
 *
 * Supports HTTP/1.1 with persistent connections, chunked transfers, 
 * cookies, redirects and multipart posts (file uploading). 
 * Automatically reconnects the socket if connection is dropped 
 * between requests. Standard HTTP authentication (Basic) is supported. 
 *
 * SSL (https) connections are supported if PHP is compiled with 
 * OpenSSL support (--with-openssl)
 *
 * Gzipped responses (Content-Encoding: gzip) is supported if PHP is
 * compiled with zlib support (--with-zlib).
 *
 * @access		public
 * @package		synd.core.lib
 */
class HttpConnection {
	/**
	 * Host to connect to
	 * @access	private
	 */
	var $_uri = null;

	/**
	 * Stored cookies, as key => value
	 * @access	private
	 */
	var $_cookies = array();
	
	/**
	 * Last response status
	 * @access	private
	 */
	var $_status = null;

	/**
	 * Last response headers
	 * @access	private
	 */
	var $_headers = array();
	
	/**
	 * Custom request headers
	 * @access	private
	 */
	var $_custom = array();
	
	/**
	 * Redirected to URI
	 * @access	private
	 */
	var $_location = null;
	
	/**
	 * Current redirect count
	 * @access	private
	 */
	var $_redirects = 0;

	/**
	 * Maximum redirect count
	 * @access	private
	 */
	var $_maxRedirects = 5;

	/**
	 * Redirect decider callback 
	 * @access	private
	 */
	var $_redirectCallback = null;
	
	/**
	 * Max response size to receive
	 * @access	private
	 */
	var $_maxResponseSize = 8388608;
	
	/**
	 * Connection timeout
	 * @access	private
	 */
	var $_connTimeout = null; 
	
	/**
	 * Socket timeout
	 * @access	private
	 */
	var $_sockTimeout = null; 

	/**
	 * @param	string	Server URI (http://www.example.com)
	 * @param	integer	Socket connection timeout in seconds
	 * @param	integer	Stream timeout in seconds
	 */
	function HttpConnection($uri, $conntimeout = 5, $socktimeout = null) {
		$this->setLocation($uri);
		$this->_connTimeout = $conntimeout;
		$this->_sockTimeout = $socktimeout;
	}
	
	/**
	 * Sets the target endpoint
	 * @param	string	Server URI (http://www.example.com)
	 */
	function setLocation($uri) {
		if (null == ($info = @parse_url($uri)) || !isset($info['scheme'], $info['host']))
			return $this->_uri = null;

		if (!isset($info['port'])) {
			switch ($info['scheme']) {
				case 'https':
					$info['port'] = 443;
					break;
				case 'http':
				default:
					$info['port'] = 80;
			}
		}

		// Clear cookies when changing hosts
		if (!empty($this->_uri) && ($info['host'] != $this->_uri['host'] || $info['port'] != $this->_uri['port']))
			$this->_cookies = array();
		
		$this->_headers = array();
		$this->_uri = $info;
	}
	
	/**
	 * Gets the current uri
	 *
	 * The current location is the host and path of the last request
	 * including any redirects that might have occurred. This 
	 * correspond to the uri that would have been displayed in a 
	 * browsers address field after completing the last request.
	 *
	 * @return	string
	 */
	function getLocation() {
		$absolute = isset($this->_uri['scheme']) ? $this->_uri['scheme'] : 'http';
		$absolute .= '://';
		
		if (isset($this->_uri['user'])) {
			$absolute .= $this->_uri['user'];
			if (isset($this->_uri['pass']))
				$absolute .= ':'.$this->_uri['pass'];
			$absolute .= '@';
		}

		$absolute .= $this->_uri['host'];
		if (isset($this->_uri['port']) && 80 != $this->_uri['port'] && 443 != $this->_uri['port'])
			$absolute .= ':'.$this->_uri['port'];
		
		$absolute .= $this->_location;
		return $absolute;
	}
	
	/**
	 * Set a callback to decide whether a redirect should be followed
	 *
	 * Format of callback is 'bool myCallback(string uri)', return if
	 * the redirect should be followed, false otherwise.
	 *
	 * @param	callback
	 */
	function setRedirectCallback($callback) {
		assert('is_callable($callback)');
		$this->_redirectCallback = $callback;
	}
	
	/**
	 * Returns the socket
	 * @access	protected
	 * @return	resource
	 */
	function getSocket($reconnect = false) {
		static $sockets = array();
		if (null == $this->_uri) {
			trigger_error('No valid URI specified.', E_USER_NOTICE);
			return false;
		}
		
		$id = "{$this->_uri['host']}:{$this->_uri['port']}";
		if (empty($sockets[$id]) || $reconnect || @feof($sockets[$id])) {
			if (!empty($sockets[$id]))
				@fclose($sockets[$id]);
			
			$host = 'https' == $this->_uri['scheme'] ? 
				"ssl://{$this->_uri['host']}" : $this->_uri['host'];
			if (false === ($socket = @fsockopen($host, $this->_uri['port'], $errno, $errstr, $this->_connTimeout))) {
//				trigger_error("Could not open socket: ($errno) $errstr.", E_USER_NOTICE);
				return false;
			}
			
			if (null != $this->_sockTimeout) 
				stream_set_timeout($socket, $this->_sockTimeout);
				
			$sockets[$id] = $socket;
		}
		
		while (count($sockets) > 10)
			@fclose(array_shift($sockets));
		
		return $sockets[$id];
	}
	
	/**
	 * Returns last request status
	 * @return	integer
	 */
	function getStatus() {
		return $this->_status;
	}
	
	/**
	 * Returns last request headers as strtolower(name) => value pairs.
	 * @return	array
	 */
	function getHeaders() {
		return $this->_headers;
	}
	
	function setRequestHeader($name, $value) {
		$this->_custom[strtolower($name)] = "$name: $value";
	}
	
	function getCookies() {
		return $this->_cookies;
	}
	
	/**
	 * GET method request. Returns the response body
	 * @param	string	Path to request
	 * @param	array	Form variables
	 * @param	string	HTTP method to use {GET, POST, HEAD, ...}
	 * @return	string
	 */
	function getRequest($path, $query = null, $method = 'GET') {
		if (null != $query)
			$path .= (false === strpos($path,'?') ? '?' : '&').http_build_query($query);
		$request = $this->_buildRequest($path, $method);
		return $this->_request($path, $request."\r\n");
	}
	
	/**
	 * POST method request. Returns the response body
	 * @param	string	Path to request
	 * @param	mixed	Request data as string or array(formvariable => value, ...)
	 * @param	string	Content-Type (application/x-www-form-urlencoded, text/xml, ...)
	 * @return	string
	 */
	function postRequest($path, $query, $type = 'application/x-www-form-urlencoded', $method = 'POST') {
		if (is_array($query))
			$query = http_build_query($query);
		$request = $this->_buildRequest($path, $method);
		$request .= "Content-Type: $type\r\n";
		$request .= "Content-Length: ".strlen($query)."\r\n";
		$request .= "\r\n";
		$request .= $query;
		return $this->_request($path, $request);
	}
	
	/**
	 * POST using multipart encoding (file uploading, ...)
	 *
	 * <code>
	 * Example of files argument
	 * array(
	 *  'file1_form_field' => 'File1 contents',
	 *  'file2_form_field' => array('file2.txt', 'File2 contents'),
	 * );
	 * </code>
	 *
	 * @param	string	Path to request
	 * @param	array	Regular form variables
	 * @param	array	Files to be posted
	 */
	function postMultipart($path, $query = array(), $files = array()) {
		$request = null;
		
		for ($i=0; $i<100; $i++) {
			$boundary = md5(uniqid(''));
			$body = '';

			foreach ($this->_implode($query) as $key => $value) {
				if (false !== strpos($key, $boundary) || 
					false !== strpos($value, $boundary))
					continue 2;

				$body .= "--$boundary\r\n";
				$body .= "Content-Disposition: form-data; name=\"$key\"\r\n\r\n";
				$body .= "$value\r\n";
			}

			foreach ($files as $key => $value) {
				$name = is_array($value) ? $value[0] : $key;
				$file = is_array($value) ? $value[1] : $value;
				
				if (false !== strpos($key, $boundary) || 
					false !== strpos($name, $boundary) || 
					false !== strpos($file, $boundary))
					continue 2;

				$body .= "--$boundary\r\n";
				$body .= "Content-Disposition: form-data; name=\"$key\"; filename=\"{$name}\"\r\n";
				$body .= "Content-Type: text/plain\r\n\r\n";
				$body .= "{$file}\r\n";
			}

			if ('' != $body)
				$body .= "--$boundary--\r\n";
			
			$request = $this->_buildRequest($path, 'POST');
			$request .= "Content-Type: multipart/form-data, boundary=$boundary\r\n";
			$request .= "Content-Length: ".strlen($body)."\r\n";
			$request .= "\r\n";
			$request .= $body;
			
			break;
		}
		
		if (null == $request)
			return trigger_error('Could not find a valid boundary to use', E_USER_WARNING);
		return $this->_request($path, $request);
	}
	
	/**
	 * HEAD method request. Returns the headers
	 * @param	string	Path to request
	 * @param	array	Form variables
	 * @return	array
	 */
	function headRequest($path, $query = null) {
		if (null != $query)
			$path .= (false === strpos($path,'?') ? '?' : '&').http_build_query($query);
		$request = $this->_buildRequest($path, 'HEAD');
		$this->_request($path, $request."\r\n", false);
		return $this->getHeaders();
	}
	
	/**
	 * @access	private
	 */
	function _implode($query, $_prefix = null) {
		$result = array();
		foreach ($query as $key => $value) {
			if (null != $_prefix)
				$key = $_prefix.'['.$key.']';
			if (is_array($value))
				$result = array_merge($result, $this->_implode($value, $key));
			else
				$result[$key] = $value;
		}
		return $result;
	}

	/**
	 * @access	private
	 */
	function _buildRequest(&$path, $method) {
		$uri = parse_url($path);
		$path = isset($uri['path']) ? $uri['path'] : '/';
		if (isset($uri['query']))
			$path .= '?'.$uri['query'];
			
		$request = "$method $path HTTP/1.1\r\n";
		$request .= "Host: {$this->_uri['host']}\r\n";
//		$request .= "Connection: close\r\n";
		if (function_exists('gzinflate')) 
			$request .= "Accept-Encoding: gzip\r\n";
		$request .= "User-Agent: Synd (http://www.synd.info/)\r\n";
		
		if (!empty($this->_custom))
			$request .= implode("\r\n", $this->_custom)."\t\n";

		if (isset($this->_uri['user']))
			$request .= "Authorization: Basic ".base64_encode("{$this->_uri['user']}:{$this->_uri['pass']}")."\r\n";
		
		if (count($this->_cookies)) {
			$request .= "Cookie: "; $i=0;
			foreach ($this->_cookies as $key => $value) {
				if ($i++) $request .= '; ';
				$request .= "$key=$value";
			}
			$request .= "\r\n";
		}
		
		return $request;
	}
	
	/**
	 * @access	protected
	 */
	function _request($path, $request, $readResponse = true) {
		$this->_location = $path;
		$length = strlen($request);

		if (false === ($socket = $this->getSocket()))
			return false;

		// Attempt to send request
		if (false === @fwrite($socket, $request, $length)) {
			// Retry once if connection closed (by server or proxy)
			if (false === ($socket = $this->getSocket(true)))
				return false;
			if (false === @fwrite($socket, $request, $length)) {
				trigger_error("Error when writing to socket ($socket).", E_USER_WARNING);
				@fclose($socket);
				return false;
			}
		}
		
		$this->_status = null;
		$this->_headers = array();
		$response = null;
		
		// Attempt to read headers
		if (false === $this->_readHeaders($socket)) {
			trigger_error("Error when reading headers from '{$this->_uri['scheme']}://{$this->_uri['host']}:{$this->_uri['port']}$path' ($socket).", E_USER_WARNING);
			return false;
		}

		$length = null;
		$encoding = null;
		$connection = false;

		if (isset($this->_headers['content-length']))
			$length = $this->_headers['content-length'];
		if (isset($this->_headers['transfer-encoding']))
			$encoding = strtolower($this->_headers['transfer-encoding']);
		if (isset($this->_headers['connection']))
			$connection = strtolower($this->_headers['connection']);
		
		if ($readResponse) {
			// Check max response size
			if (is_numeric($length) && $length >= $this->_maxResponseSize) {
				fclose($socket);
				return false;
			}

			// Read response body
			if (preg_match('/\bchunked\b/i', $encoding)) {
				// Assemble response from chunks
				if (false === ($response = $this->_readChunkedResponse($socket)))
					return false;
				// Read optional footers
				$this->_readHeaders($socket);
			}
			else {
				// If 'Content-Length' recieved
				if (is_numeric($length)) {
					if (false === ($response = $this->_read($socket, $length)))
						return false;
				}
				else {
					while (!feof($socket)) {
						if (false === ($response .= fread($socket, 8192)) ||
							strlen($response) >= $this->_maxResponseSize) {
							@fclose($socket);
							return false;
						}
					}
				}
			}

			// Inflate gzipped response
			if (isset($this->_headers['content-encoding']) && 'gzip' == $this->_headers['content-encoding']) {
				if (!function_exists('gzinflate')) {
					trigger_error("Unsupported response encoding: 'gzip'", E_USER_WARNING);
					return $this->disconnect();
				}
				$response = gzinflate(substr($response, 10));
			}
		}

		// If 'Connection: close' recieved
		if ('close' == $connection || 5 == substr($this->_status, 0, 1))
			while (!feof($socket) && false !== $this->_read($socket, 8192));

		// Check for redirection
		if (empty($this->_headers['location']))
			$this->_redirects = 0;
		else if ($readResponse) {
			if ($this->_redirects >= $this->_maxRedirects)
				trigger_error("Maximun number of redirects {$this->_maxRedirects} reached.", E_USER_WARNING);
			else if ((
				null === $this->_redirectCallback || 
				call_user_func($this->_redirectCallback, $this->_headers['location'])) &&
				false !== ($nextResponse = $this->_redirect($path, $this->_headers['location'])))
				return $nextResponse;
		}
		
		return $response;
	}
	
	/**
	 * Resolves a relative path into an absolute
	 * @param	string	Current path
	 * @param	string	Relative path
	 * @return	string
	 */
	function relative($current, $relative) {
		if ('//' == substr($relative,0,2))
			return $this->_uri['scheme'].':'.$relative;
		
		$absolute = isset($this->_uri['scheme']) ? $this->_uri['scheme'] : 'http';
		$absolute .= '://';

		if (isset($this->_uri['user'])) {
			$absolute .= $this->_uri['user'];
			if (isset($this->_uri['pass']))
				$absolute .= ':'.$this->_uri['pass'];
			$absolute .= '@';
		}
		
		$absolute .= $this->_uri['host'];
		if (isset($this->_uri['port']) && 80 != $this->_uri['port'] && 443 != $this->_uri['port'])
			$absolute .= ':'.$this->_uri['port'];
		
		if ('/' == $relative{0})
			$absolute .= $relative;
		else {
			$uri = parse_url($current);
			if (!isset($uri['path']))
				$absolute .= '/'.$relative;
			else if ('/' == substr($uri['path'], -1))
				$absolute .= $uri['path'].$relative;
			else {
				$absolute .= dirname($uri['path']);
				if ('/' != substr($absolute, -1))
					$absolute .= '/';
				$absolute .= $relative;
			}
		}

		return str_replace(array('/./','/../'), '/', 
			preg_replace('/(?<!:\/)\/[^\/]*(\/|(?R))+\.\./S', '', $absolute));
	}
	
	/**
	 * Performs a redirection
	 * @access	private
	 */
	function _redirect($current, $location) {
		$uri = false !== ($i = strpos($location,'://')) && $i < 6 ? $location : $this->relative($current, $location);
		$this->_redirects++;
		$this->setLocation($uri);
		return $this->getRequest($uri);
	}
	
	/**
	 * @access	private
	 */
	function _readHeaders($socket) {
		while (!feof($socket)) {
			if (false === ($line = $this->_gets($socket)))
				return false;
			if ("\r\n" === $line || "\n" === $line)
				break;
				
			// Match status header
			if (!preg_match('/^HTTP\/\d+\.\d+ (\d+)/i', $line, $matches))
				$this->_header($line);
			// Skip "100 Continue" response
			else if ('1' == substr($matches[1], 0, 1))
				$this->_readHeaders($socket);
			else
				$this->_status = $matches[1];
		}
		return true;
	}
	
	function _header($line) {
		if (false !== ($i = strpos($line, ':'))) {
			$name = substr($line, 0, $i);
			$value = substr($line, $i+1);
			if ('set-cookie' == ($name = strtolower($name)))
				$this->_cookies = array_merge($this->_cookies, $this->_parseCookie($value));
			$this->_headers[$name] = trim($value);
		}
	}
	
	/**
	 * Reads a chunked response from a socket
	 * @access	private
	 */
	function _readChunkedResponse($socket) {
		$response = null;
		$length = null;
		
		while (!feof($socket)) {
			// Read and decode chunk length
			$line = $this->_gets($socket);
			if (!preg_match('/^([0-9a-f]+)/iS', $line, $match)) {
				trigger_error('Error when parsing chunk length.', E_USER_WARNING);
				return false;
			}

			// 0 indicates last chunk
			if (0 == ($length = hexdec($match[1])))
				break;
			
			// Check max response size
			if (strlen($response) + $length >= $this->_maxResponseSize) {
				fclose($socket);
				return false;
			}

			// Read chunk data and trailing \r\n
			if (false === ($response .= $this->_read($socket, $length)) ||
				false === $this->_gets($socket)) 
				return false;
		}
		
		return $response;
	}
	
	/**
	 * @access	private
	 */
	function _gets($socket) {
		$response = null;
		while (!feof($socket) && "\n" != substr($response, -1)) {
			if (false === ($chunk = fgets($socket, 8192))) {
				@fclose($socket);
				return false;
			}
			$response .= $chunk;
		}
		return $response;
	}
	
	/**
	 * @access	private
	 */
	function _read($socket, $length) {
		$response = null;
		while (!feof($socket) && $length > 0) {
			if (false === ($chunk = fread($socket, $length))) {
				@fclose($socket);
				return false;
			}
			
			$length -= strlen($chunk);
			$response .= $chunk;
		}
		return $response;
	}
	
	/**
	 * @access	private
	 */
    function _parseCookie($value) {
		$value = explode(';', $value);
		$value = explode('=', $value[0]);
		return array(trim($value[0]) => trim($value[1]));
    }
}
