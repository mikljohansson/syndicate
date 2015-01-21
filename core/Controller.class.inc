<?php
require_once 'core/lib/SyndLib.class.inc';

/**
 * Front controller interface
 * @access		public
 * @package		synd.core
 */
interface Controller {
	/**
	 * Serves a request
	 * @throws	RuntimeException
	 * @param	Request	Request to process
	 * @return	mixed
	 */
	function request(Request $request);
}

/**
 * Abstract front controller
 * 
 * Attempts to delegate requests to _view_* methods on this object
 *
 * @access		public
 * @package		synd.core
 */
abstract class AbstractController implements Controller {
	function request(Request $request) {
		global $synd_config;
		try {
			$method = empty($request[0]) ? '_view_index' : '_view_'.strtr($request[0], '.', '_');
			if (method_exists($this, $method))
				return call_user_func(array($this, $method), $request->forward(), new Template(array_reverse($synd_config['dirs']['design']), $request));
			if (!empty($request[0]) && null != ($instance = SyndLib::getInstance($request[0])) && ($instance instanceof Controller))
				return $instance->request($request->forward());
		}
		catch (PartialContentException $e) {
			for ($i=ob_get_level(); $i > 0; $i--)
				ob_end_clean();
			return array('content' => $e->getContent(), 'page' => null);
		}
		return false;
	}
}

abstract class PublishedController extends AbstractController {
	protected static $_instances = array();
	protected $_id = null;
	
	function __construct() {
		self::$_instances[$this->_id = md5(uniqid(''))] = $this;
	}
	
	function __wakeup() {
		self::$_instances[$this->_id] = $this;
	}
	
	function __clone() {
		self::$_instances[$this->_id = md5(uniqid(''))] = $this;
	}
	
	function id() {
		return $this->_id;
	}
	
	static function __callback_instance(&$result, $id) {
		if (isset(self::$_instances[$id])) {
			$result = self::$_instances[$id];
			return true;
		}
	}
}

SyndLib::attachHook('instance', array('PublishedController', '__callback_instance'), -2500);

/**
 * Accepts requests and passes them onto a proper controller
 * @access		public
 * @package		synd.core
 */
class RequestProcessor {
	/**
	 * @throws	RuntimeException
	 * @param	Request	Request to process
	 * @return	mixed
	 */
	function request(Request $request) {
		if (isset($request[0]) && null != ($instance = SyndLib::getInstance($request[0])) && ($instance instanceof Controller))
			return $instance->request($request->forward());
		else if (null != ($instance = SyndLib::runHook('default_request_handler')))
			return $instance->request($request);
		return false;
	}
}

/**
 * Encapsulates a single request
 * @access		public
 * @package		synd.core
 */
class Request implements IteratorAggregate, ArrayAccess {
	public $session;
	public $uribase;
	public $context;
	public $request;
	public $files;
	
	protected $_context;
	protected $_path;

	function __construct(&$session, $uribase = '/', $context = '', $request = array(), $files = array()) {
		$this->session =& $session;
		$this->uribase = $uribase;
		
		$ctx = trim($context, '/');
		$this->context = $this->_context = ($ctx != '' ? explode('/', $ctx) : array());
		
		$this->request = $request;
		$this->files = $files;
		$this->_path = $uribase.$context;
	}

	function getIterator() {
		return new ArrayIterator($this->request);
	}
	
	function offsetExists($offset) {
		return isset($this->_context[$offset]) || isset($this->request[$offset]) || isset($this->files[$offset]);
	}

	function offsetGet($offset) {
		if (isset($this->_context[$offset]))
			return $this->_context[$offset];
		if (isset($this->request[$offset]))
			return $this->request[$offset];
		if (isset($this->files[$offset]))
			return $this->files[$offset];
		return null;
	}

	function offsetSet($offset, $value) {
		if (is_int($offset))
			$this->_context[$offset] = $value;
		else
			$this->request[$offset] = $value;
	}
	
	function offsetUnset($offset) {
		if (isset($this->_context[$offset]))
			unset($this->_context[$offset]);
	}
	
	/**
	 * Shifts the first component off the context path and returns this
	 * @return	Request
	 */
	function forward() {
		array_shift($this->_context);
		return $this;
	}
	
	function getContext() {
		return $this->_context;
	}
	
	function setContext($context) {
		$this->_context = $context;
	}
	
	function getVirtualPath() {
		return $this->_path;
	}
}

/**
 * Response to a request
 */
class Response {
	protected $_content = null;
	protected $_location = null;
	
	function setContent($content) {
		$this->_content = $content;
	}
	
	function setLocation($location) {
		$this->_location = $location;
	}
	
	function toArray() {
		return array('content' => $this->_content, 'redirect' => $this->_location);
	}
}

class HttpRequest extends Request {
	public $get;
	public $post;
	public $cookie;
	
	/**
	 * @param	array	Session array, $_SESSION
	 * @param	string	URI base, '/synd/' when using mod_rewrite or '/synd/index.php?sq=' when not
	 * @param	string	Context path, $_GET['sq']
	 * @param	array	Request parameters, $_REQUEST
	 * @param	array	Query string variables, $_GET
	 * @param	array	Form variables, $_POST
	 * @param	array	Uploaded files, SyndLib::filesTransform($_FILES)
	 * @param	array	Cookie variables, $_COOKIE
	 */
	function __construct(&$session, $uribase = '/', $context = '', $request = array(), $get = array(), $post = array(), $files = array(), $cookie = array()) {
		parent::__construct($session, $uribase, $context, $request, $files);
		$this->get = $get;
		$this->post = $post;
		$this->cookie = $cookie;
	}
}

/**
 * @package		synd.core
 */
class AuthorizationException extends RuntimeException {}

/**
 * @package		synd.core
 */
class ForbiddenException extends RuntimeException {}

/**
 * @package		synd.core
 */
class NotFoundException extends RuntimeException {}

/**
 * Output partial content 
 *
 * Used to bypass futher request processing and output a single page 
 * fragment, for example when refreshing a partial page using AJAX
 *
 * @package		synd.core
 */
class PartialContentException extends RuntimeException {
	protected $_content = null;
	
	function __construct($content) {
		parent::__construct('Partial content refresh');
		$this->_content = $content;
	}
	
	function getContent() {
		return $this->_content;
	}
}
