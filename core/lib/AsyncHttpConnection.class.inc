<?php
require_once 'core/lib/HttpConnection.class.inc';
global $_synd_async_instances, $_synd_async_sending, $_synd_async_receiving;

/**
 * Connection instance (socket resource number as key)
 * @var		array
 * @access	private
 */
$_synd_async_instances = array();

/**
 * Sending sockets
 * @var		array
 * @access	private
 */
$_synd_async_sending = array();

/**
 * Receiving sockets
 * @var		array
 * @access	private
 */
$_synd_async_receiving = array();

/**
 * Async HTTP response callback interface
 * @abstract
 * @access		public
 * @package		synd.core.lib
 */
class ISyndAsyncHTTPCallback {
	/**
	 * Callback method that receives HTTP responses
	 * @param	AsyncHttpConnection	The receiving connection
	 * @param	string	The response body
	 */
	function httpResponse($connection, $response) {}

	/**
	 * Callback method that receives HTTP errors
	 * @param	AsyncHttpConnection	The receiving connection
	 */
	function httpError($connection) {}
}

/**
 * Asyncronous HTTP/1.1 connection
 *
 * Register a callback to receive respones when creating the 
 * connection. Calling the static select() method will wait on all
 * connections for responses to be received. 
 *
 * See the SyndSpider class for a more extensive example
 *
 * <code>
 *  class MyResponseCallback {
 *  	function httpResponse($connection, $response) {
 *			print $response;
 *		}
 *
 *		function httpError($connection) {
 *			trigger_error('HTTP ('.$connection->getStatus().') error received', E_USER_WARNING);
 *		}
 *  }
 * 
 *  $connection = new AsyncHttpConnection('http://www.example.com', new MyResponseCallback());
 *  $connection->getRequest('/index.html');
 *
 *  for ($i=0; $i<5; $i++) {
 *  	AsyncHttpConnection::select();
 *  	usleep(25000);
 *  }
 * </code>
 *
 * @access		public
 * @package		synd.core.lib
 * @see			SyndSpider
 */
class AsyncHttpConnection extends HttpConnection {
	/**
	 * @access	private
	 */
	var $_id = null;

	/**
	 * @access	protected
	 */
	var $_socket = null;

	/**
	 * @var		object	ISyndAsyncHTTPCallback
	 * @access	protected
	 */
	var $_callback = null;

	/**
	 * Send channel state
	 * @var		object	IAsyncHTTPState
	 * @access	protected
	 */
	var $_send = null;
	
	/**
	 * Receive channel state
	 * @var		object	IAsyncHTTPState
	 * @access	protected
	 */
	var $_recv = null;
	
	/**
	 * Request send buffer
	 * @access	protected
	 */
	var $_request = null;
	
	/**
	 * Response receive buffer
	 * @access	protected
	 */
	var $_response = null;

	/**
	 * Read entire response (HEAD requests don't)
	 * @access	private
	 */
	var $_readResponse = true;
	
	/**
	 * Receive buffer used by _gets() and _read()
	 * @access	private
	 */
	var $_buffer = null;

	/**
	 * Chunked response length
	 * @access	private
	 */
	var $_length = null;
	
	/**
	 * Microtime when the connection went idle
	 * @var		string
	 * @see		microtime()
	 * @access	private
	 */
	var $_idle = null;
	
	/**
	 * Time of last send/recv activity
	 * @var		integer
	 * @access	private
	 */
	var $_lastActivity = 0;
	
	/**
	 * @access	private
	 */
	var $_idleState = null;
	
	/**
	 * @access	private
	 */
	var $_sendRequestState = null;

	/**
	 * @access	private
	 */
	var $_recvHeadersState = null;

	/**
	 * @access	private
	 */
	var $_recvContinueState = null;

	/**
	 * @access	private
	 */
	var $_recvOptionalHeadersState = null;

	/**
	 * @access	private
	 */
	var $_recvResponseState = null;

	/**
	 * @access	private
	 */
	var $_recvLengthState = null;

	/**
	 * @access	private
	 */
	var $_recvChunkedState = null;

	/**
	 * @access	private
	 */
	var $_recvConnectionClose = null;
	
	/**
	 * @param	string	Server URI (http://www.example.com)
	 * @param	ISyndAsyncHTTPCallback	Response callback
	 */
	function AsyncHttpConnection($uri, $callback) {
		parent::HttpConnection($uri);
		$this->_id = md5(uniqid(''));
		$this->_callback = $callback;
		
		// Instantiate connection states
		$this->_idleState = new AsyncHTTPIdle();
		$this->_sendRequestState = new AsyncHTTPSendRequest();

		$this->_recvHeadersState 	= new AsyncHTTPReceiveHeaders();
		$this->_recvContinueState 	= new AsyncHTTPReceiveContinue();
		$this->_recvOptionalHeadersState = new AsyncHTTPReceiveOptionalHeaders();

		$this->_recvResponseState	= new AsyncHTTPReceiveResponse();
		$this->_recvLengthState 	= new AsyncHTTPReceiveLengthResponse();
		$this->_recvChunkedState	= new AsyncHTTPReceiveChunkedResponse();
		$this->_recvConnectionClose	= new AsyncHttpConnectionClose();
		
		$this->_send = $this->_idleState;
		$this->_recv = $this->_idleState;
		$this->_idle = microtime();
	}

	/**
	 * Wait for responses to become available
	 * 
	 * Typically the script will schedule some HTTP requests with one
	 * or more connections and then call select() in a loop to wait for 
	 * the responses to be received (use of usleep(10000) is recommended)
	 *
	 * @param	integer	The maxiumum number of microseconds to wait for data
	 */
	static function select($timeout = 250000) {
		global $_synd_async_instances, $_synd_async_sending, $_synd_async_receiving;
//		profiler_enter(__METHOD__);
		
		$send = $_synd_async_sending;
		$recv = $_synd_async_receiving;
		$oob = null;

		if (!empty($send) || !empty($recv)) {
			stream_select($recv, $send, $oob, 0, $timeout);
			$time = time();
			
			foreach ($send as $socket) {
				if (isset($_synd_async_instances[$socket])) {
					$_synd_async_instances[(int)$socket]->_lastActivity = $time;
					$_synd_async_instances[(int)$socket]->_send->send($_synd_async_instances[(int)$socket]);
				}
			}

			foreach ($recv as $socket) {
				if (isset($_synd_async_instances[(int)$socket])) {
					$_synd_async_instances[(int)$socket]->_lastActivity = $time;
					$_synd_async_instances[(int)$socket]->_recv->receive($_synd_async_instances[(int)$socket]);
				}
			}
			
			// Check timeout for sockets without activity
			foreach (array_unique(array_merge(
				array_diff($_synd_async_sending, $send),
				array_diff($_synd_async_receiving, $recv))) as $socket) {
				if (false === $_synd_async_instances[(int)$socket]->_idle && 
					$_synd_async_instances[(int)$socket]->_lastActivity < $time - $_synd_async_instances[(int)$socket]->_sockTimeout)
					$_synd_async_instances[(int)$socket]->_error();
			}
		}
		
//		profiler_leave(__METHOD__);
	}

	/**
	 * Disconnects the connection and deallocates resources
	 */
	function _disconnect() {
		global $_synd_async_instances, $_synd_async_sending, $_synd_async_receiving;
		unset($_synd_async_sending[$this->_id]);
		unset($_synd_async_receiving[$this->_id]);

		if (null != $this->_socket) {
			unset($_synd_async_instances[(int)$this->_socket]);
			@fclose($this->_socket);
			$this->_socket = null;
		}
		
		$this->_send = $this->_idleState;
		$this->_recv = $this->_idleState;
	}

	/**
	 * Returns the socket
	 * @access	protected
	 * @return	resource
	 */
	function _connect($reconnect = false) {
		global $_synd_async_instances;
		if (null == $this->_uri) {
			trigger_error('No valid URI specified.', E_USER_NOTICE);
			return false;
		}
		
		if ($reconnect || null == $this->_socket || @feof($this->_socket)) {
			if (null != $this->_socket) {
				unset($_synd_async_instances[(int)$this->_socket]);
				@fclose($this->_socket);
			}
			
			$host = 'https' == $this->_uri['scheme'] ? 
				"ssl://{$this->_uri['host']}" : $this->_uri['host'];
			if (false === ($this->_socket = @fsockopen($host, $this->_uri['port'], $errno, $errstr, 5))) {
//				trigger_error("Could not open socket: ($errno) $errstr.", E_USER_NOTICE);
				return false;
			}
			
			if (!stream_set_blocking($this->_socket, false)) {
				trigger_error('Could not set socket non-blocking.', E_USER_WARNING);
				return false;
			}

//			if (null != $this->_sockTimeout)
//				stream_set_timeout($this->_socket, $this->_sockTimeout);

			$_synd_async_instances[(int)$this->_socket] = $this;
		}
		
		return true;
	}

	function _error() {
		$this->_disconnect();
		$this->_callback->httpError($this);
	}
	
	/**
	 * @access	protected
	 */
	function _request($path, $request, $readResponse = true) {
		global $_synd_async_sending, $_synd_async_receiving;
		
		$this->_location = $path;
		$this->_status = null;
		$this->_headers = array();
		
		$this->_readResponse = $readResponse;
		$this->_response = null;
		
		$this->_buffer = null;
		$this->_idle = false;

		$this->_size = strlen($request);
		$this->_offset = 0;
		$this->_request = $request;

		if ($this->_connect()) {
			$this->_send = $this->_sendRequestState;
			$this->_recv = $this->_recvHeadersState;

			$_synd_async_sending[$this->_id] = $this->_socket;
			$_synd_async_receiving[$this->_id] = $this->_socket;
		}
	}
	
	/**
	 * Returns idle time with microseconds, false if connection is busy
	 * @return	float
	 */
	function getIdletime() {
		if (false === $this->_idle)
			return false;
		list ($s1, $m1) = explode(' ', $this->_idle);
		list ($s2, $m2) = explode(' ', microtime());
		return $s2 + $m2 - $s1 - $m1;
	}
	
	/**
	 * Reads a line from socket, returns null if no full line currently available
	 * @access	private
	 */
	function _gets($socket) {
		while (false === ($i = strpos($this->_buffer, "\n")) && !feof($socket) &&
			null != ($this->_buffer .= fread($socket, 8192)));
		
		if (false !== $i) {
			$line = substr($this->_buffer, 0, $i+1);
			$this->_buffer = substr($this->_buffer, $i+1);
			return $line;
		}

		return null;
	}
	
	/**
	 * Reads from socket, returns null if full $length not available
	 * @access	private
	 */
	function _read($socket, $length) {
		while (strlen($this->_buffer) < $length && !feof($socket) &&
			null != ($this->_buffer .= fread($socket, $length - strlen($this->_buffer))));
		
		if (strlen($this->_buffer) >= $length) {
			$chunk = substr($this->_buffer, 0, $length);
			$this->_buffer = substr($this->_buffer, $length);
			return $chunk;
		}
		
		return null;
	}
}

/**
 * Async HTTP connection state
 * @abstract
 * @access	private
 */
class IAsyncHTTPState {
	/**
	 * Called from select() when data can be sent
	 */
	function send($connection) {
		assert(false);
	}

	/**
	 * Called from select() on incoming data
	 */
	function receive($connection) {
		assert(false);
	}
}

/**
 * Idle state; will discard any received packets
 * @access	private
 */
class AsyncHTTPIdle extends IAsyncHTTPState {
	function send($connection) {
	}

	function receive($connection) {
		while (!feof($connection->_socket) && null != fread($connection->_socket, 8192));
	}
}

/**
 * Send request state
 * @access	private
 */
class AsyncHTTPSendRequest extends IAsyncHTTPState {
	function send($connection) {
		global $_synd_async_sending;
//		profiler_enter(__METHOD__);

		// Retry once if connection closed (by server or proxy)
		if (false === ($connection->_offset += fwrite($connection->_socket, 
			substr($connection->_request, $connection->_offset)))) {

			$connection->_offset = 0;
			$connection->_connect(true);
			
			if (false === ($connection->_offset += fwrite($connection->_socket, 
				substr($connection->_request, $connection->_offset)))) {
				trigger_error("Error sending request to '{$connection->_uri['scheme']}://{$connection->_uri['host']}:{$connection->_uri['port']}$connection->_location'.", E_USER_NOTICE);
				$connection->_error();
			}
		}
		
		if ($connection->_offset >= strlen($connection->_request)) {
			$connection->_send = $connection->_idleState;
			unset($_synd_async_sending[$connection->_id]);
		}

//		profiler_leave(__METHOD__);
	}
}

/**
 * Receive state
 * @access	private
 */
class AsyncHTTPReceive extends IAsyncHTTPState {
	function _response($connection) {
		global $_synd_async_receiving;
		
		// Check for redirection
		if (empty($connection->_headers['location']))
			$connection->_redirects = 0;
		else if ($connection->_redirects >= $connection->_maxRedirects)
			trigger_error("Maximun number of redirects {$connection->_maxRedirects} reached.", E_USER_WARNING);
		else if (null == $connection->_redirectCallback || 
			call_user_func($connection->_redirectCallback, $connection->_headers['location']))
			return $connection->_redirect($connection->_location, $connection->_headers['location']);
			
		// Inflate gzipped response
		if (isset($connection->_headers['content-encoding']) && 
			'gzip' == $connection->_headers['content-encoding']) {
			if (function_exists('gzinflate')) 
				$connection->_response = gzinflate(substr($connection->_response, 10));
			else {
				trigger_error("Unsupported response encoding: 'gzip'", E_USER_WARNING);
				return $connection->_error();
			}
		}

		unset($_synd_async_receiving[$connection->_id]);
		$connection->_recv = $connection->_idleState;
		$connection->_idle = microtime();
		
		$connection->_callback->httpResponse($connection, $connection->_response);
	}
	
	function _next($connection) {
		if (!isset($connection->_headers['connection']) ||
			'close' != strtolower($connection->_headers['connection']))
			$this->_response($connection);
		else {
			$connection->_recv = $connection->_recvConnectionClose;
			$connection->_recv->receive($connection);
		}
	}
}

/**
 * Receive response headers
 * @access	private
 */
class AsyncHTTPReceiveHeaders extends AsyncHTTPReceive {
	function receive($connection) {
//		profiler_enter(__METHOD__);
		
		while (null != ($line = $connection->_gets($connection->_socket))) {
			if ("\r\n" !== $line && "\n" !== $line) {
				// Match status header
				if (!preg_match('/^HTTP\/\d+\.\d+ (\d+)/i', $line, $matches))
					$connection->_header($line);
				// Skip "100 Continue" response
				else if ('1' == substr($matches[1], 0, 1)) {
					$connection->_recv = $connection->_recvContinueState;
					$connection->_recv->receive($connection);
					break;
				}
				else
					$connection->_status = $matches[1];
			}
			else {
				// Extract Content-Length
				$connection->_length = isset($connection->_headers['content-length']) && 
					is_numeric($connection->_headers['content-length']) ? 
					(int)$connection->_headers['content-length'] : null;
				
				// Check max response size
				if ($connection->_length >= $connection->_maxResponseSize) {
					$connection->_error();
					break;
				}

				// HEAD requests don't have a response body
				if (!$connection->_readResponse || 0 === $connection->_length)
					$this->_response($connection);
				else {
					if (isset($connection->_headers['transfer-encoding']) && 
						preg_match('/\bchunked\b/i', $connection->_headers['transfer-encoding'])) {
						$connection->_recv = $connection->_recvChunkedState;
						$connection->_length = null;
					}
					else if (null != $connection->_length)
						$connection->_recv = $connection->_recvLengthState;
					else 
						$connection->_recv = $connection->_recvResponseState;
				
					$connection->_recv->receive($connection);
				}
				
				break;
			}
		}
		
//		profiler_leave(__METHOD__);
	}
}

/**
 * Post chunked response optional headers
 * @access	private
 */
class AsyncHTTPReceiveOptionalHeaders extends AsyncHTTPReceive {
	function receive($connection) {
//		profiler_enter(__METHOD__);

		while (null != ($line = $connection->_gets($connection->_socket))) {
			if ("\r\n" !== $line && "\n" !== $line) 
				$connection->_header($line);
			else {
				$this->_next($connection);
				break;
			}
		}
				
//		profiler_leave(__METHOD__);
	}
}

/**
 * Skips 100 Continue responses and passes control back to ReceiveHeaders
 * @access	private
 */
class AsyncHTTPReceiveContinue extends AsyncHTTPReceiveOptionalHeaders {
	function _next($connection) {
		$connection->_recv = $connection->_recvHeadersState;
		$connection->_recv->receive($connection);
	}
}

/**
 * No Content-Length: or chunked response; read until feof()
 * @access	private
 */
class AsyncHTTPReceiveResponse extends AsyncHTTPReceive {
	function receive($connection) {
//		profiler_enter(__METHOD__);

		while (!feof($connection->_socket) && 
			null != ($connection->_response .= fread($connection->_socket, 8192))) {
			if (strlen($connection->_response) >= $connection->_maxResponseSize)
				return $connection->_error();
		}
		
		if (feof($connection->_socket)) {
			$connection->_disconnect();
			$this->_response($connection);
		}

//		profiler_leave(__METHOD__);
	}
}

/**
 * Content-Length: 123
 * @access	private
 */
class AsyncHTTPReceiveLengthResponse extends AsyncHTTPReceive {
	function receive($connection) {
//		profiler_enter(__METHOD__);

		if (null != ($connection->_response = 
			$connection->_read($connection->_socket, $connection->_length))) 
			$this->_next($connection);

//		profiler_leave(__METHOD__);
	}
}

/**
 * Transfer-Encoding: chunked
 * @access	private
 */
class AsyncHTTPReceiveChunkedResponse extends AsyncHTTPReceive {
	function receive($connection) {
//		profiler_enter(__METHOD__);

		for (;;) {
			// Read and decode chunk length
			if (null == $connection->_length) {
				if (null == ($line = $connection->_gets($connection->_socket)))
					break;
					
				if (!preg_match('/^([0-9a-f]+)/iS', $line, $matches)) {
					trigger_error("Error parsing chunk length from '{$connection->_uri['scheme']}://{$connection->_uri['host']}:{$connection->_uri['port']}$connection->_location' (".substr($line,0,25).")", E_USER_WARNING);
					$connection->_error();
					break;
				}

				// 0 indicates last chunk
				if (0 == ($connection->_length = hexdec($matches[1]))) {
					$connection->_recv = $connection->_recvOptionalHeadersState;
					$connection->_recv->receive($connection);
					break;
				}

				// Check max response size
				if (strlen($connection->_response) + $connection->_length >= $connection->_maxResponseSize) {
					$connection->_error();
					break;
				}
			}
			
			// Read chunk data
			if ($connection->_length && null == ($connection->_response .= 
				$connection->_read($connection->_socket, $connection->_length)))
				break;

			// Read trailing \r\n
			$connection->_length = 0;
			if (null == $connection->_gets($connection->_socket))
				break;

			// Read next chunk length
			$connection->_length = null;
		}
		
//		profiler_leave(__METHOD__);
	}
}

/**
 * Connection: close 
 * @access	private
 */
class AsyncHttpConnectionClose extends AsyncHTTPReceive {
	function receive($connection) {
//		profiler_enter(__METHOD__);
		while (!feof($connection->_socket) && null != fread($connection->_socket, 8192));
		
		if (feof($connection->_socket)) {
			$connection->_disconnect();
			$this->_response($connection);
		}

//		profiler_leave(__METHOD__);
	}
}
