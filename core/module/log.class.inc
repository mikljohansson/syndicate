<?php
require_once 'core/Module.class.inc';

/**
 * Error logger and watchdog module
 *
 * Logs and displays application errors with location, stacktrace 
 * and context. Logs HTTP errors with stacktrace, post data and 
 * referrer.
 *
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_log extends Module {
	var $_enabled = true;
	var $_count = 0;
	var $_title = null;
	
	function __construct() {
		global $synd_error_handler;
		if (!isset($synd_error_handler) /*&& isset($_SERVER['REMOTE_ADDR'])*/) {
			error_reporting(E_ALL & ~E_NOTICE & ~E_USER_NOTICE | E_STRICT);
			set_error_handler(array($this, '_callback_error_handler'));
		}
		
		SyndLib::attachHook('menu_admin', array($this, '_callback_menu_admin'));
	}

	/**
	 * @access	protected
	 */
	function _hook_manage_permissions(&$result) {
		$result[] = $this;	
	}

	function getDescription() {
		return SyndLib::translate('Logs application exceptions and HTTP errors.');
	}
	
	function _callback_menu_admin(&$result, $menu) {
		if ($this->isPermitted('admin'))
			$menu->append(tpl_link('system','log'), SyndLib::translate('Log'));
	}

	function _hook_http_error(&$result, $status, $uri, $exception = null) {
		global $synd_maindb, $synd_user;
		if ($this->_enabled) {
			$this->_enabled = false;

			if (1 == rand(1,250))
				$this->_gc();

			if (null == $uri)
				$uri = $_SERVER['REQUEST_URI'];

			$data = array(
				'HASH' 			=> md5("$uri:$status"),
				'TIMESTAMP' 	=> time(),
				'FILENAME'		=> $exception->getFile(),
				'LINE'			=> $exception->getLine(),
				'STATUS' 		=> $status,
				'CODE'			=> $exception->getCode(),
				'MESSAGE'		=> $exception->getMessage(),
				'DESCRIPTION'	=> method_exists($exception, 'getDescription') ? $exception->getDescription() : null,
				'CONTEXT'		=> null,
				'STACKTRACE' 	=> $exception->getTraceAsString(),
				'CLIENT_NODE_ID'=> isset($synd_user) ? $synd_user->id() : null,
				'REQUEST_URI'	=> $uri,
				'REQUEST_DATA'	=> $synd_maindb->wrapBinary(serialize($_REQUEST)),
				'REQUEST_ENV'	=> $synd_maindb->wrapBinary(serialize($_SERVER)),
				);
				
			try {
				$synd_maindb->replace('synd_log', $data);
			}
			catch (DatabaseException $e) {
				error_log($e->getMessage());
			}

			$this->_enabled = true;
		}
	}
	
	function _callback_error_handler($errno, $errstr, $errfile, $errline, $errcontext) {
		global $synd_maindb, $synd_user, $synd_error_triggered;
		if ($this->_enabled && (error_reporting() & $errno)) {
			$this->_enabled = false;
			$synd_error_triggered = true;

			// Ignore limit and errors suppressed using '@'
			if ($this->_count++ > 5) {
				$this->_enabled = true;
				return;
			}

			if (1 == rand(1,250))
				$this->_gc();

			// Log to error_log
			if (ini_get('log_errors') || ini_get('display_errors')) {
				switch ($errno) {
					case E_ERROR:
					case E_USER_ERROR:
						$logline = "Error: ";
						break;

					case E_WARNING:
					case E_USER_WARNING:
						$logline = "Warning: ";
						break;

					case E_NOTICE:
					case E_USER_NOTICE:
						$logline = "Notice: ";
						break;

					default:
						$logline = "Unknown ($errno): ";
						break;
				}

				$logline = "$errstr in $errfile on line $errline";
				$logline = preg_replace('/\s+/', ' ', $logline);

				if (ini_get('log_errors'))
					error_log($logline);
				if (ini_get('display_errors'))
					print $logline;
			}

			$stacktrace = debug_backtrace();
			array_shift($stacktrace);

			// Force clone of context
			$context = null;
			if (is_array($errcontext)) {
				$context = array_slice($errcontext,0);
				unset($context['GLOBALS']);
			}

			$uri = isset($_SERVER['REQUEST_URI']) ? $_SERVER['REQUEST_URI'] : '';
			$root = isset($_SERVER['DOCUMENT_ROOT']) ? $_SERVER['DOCUMENT_ROOT'] : null;

			$data = array(
				'HASH' 			=> md5("$errno:$errfile:$errline:".substr($errstr,0,10).":$uri"),
				'TIMESTAMP' 	=> time(),
				'FILENAME' 		=> null != $root ? str_replace($root,'',$errfile) : $errfile,
				'LINE' 			=> $errline,
				'STATUS'		=> null,
				'CODE' 			=> $errno,
				'MESSAGE' 		=> $errstr,
				'DESCRIPTION'	=> null,
				'CONTEXT' 		=> $this->_context($context),
				'STACKTRACE' 	=> $this->_trace($stacktrace),
				'CLIENT_NODE_ID'=> isset($synd_user) ? $synd_user->id() : null,
				'REQUEST_URI'	=> $uri,
				'REQUEST_DATA'	=> $synd_maindb->wrapBinary(serialize($_REQUEST)),
				'REQUEST_ENV'	=> $synd_maindb->wrapBinary(serialize($_SERVER)),
				);

			try {
				$synd_maindb->replace('synd_log', $data);
			}
			catch (DatabaseException $e) {
				error_log($e->getMessage());
			}

			if (E_ERROR == $errno ||  E_USER_ERROR == $errno) {
				if (!headers_sent() && isset($_SERVER['REMOTE_ADDR']))
					header('HTTP/1.0 500 error: Internal Server Error');
				exit(1);
			}

			$this->_enabled = true;
		}
	}
	
	function getErrors($offset = 0, $limit = 30, $order = null) {
		global $synd_maindb;
		if (null == $order)
			$order = array('TIMESTAMP', false);
		
		$sql = "
			SELECT e.HASH, e.FILENAME, e.LINE, e.REQUEST_URI, e.CODE, e.STATUS, MAX(e.timestamp) TIMESTAMP
			FROM synd_log e
			GROUP BY e.hash, e.filename, e.line, e.request_uri, e.code, e.status
			ORDER BY ".tpl_sort_sql($order);
		$errors = $synd_maindb->getAll($sql, $offset, $limit);
		
		if (!empty($errors)) {
			$sql = "
				SELECT e.hash, e.message FROM synd_log e
				WHERE e.hash IN (".implode(',', $synd_maindb->quote(SyndLib::array_collect($errors,'HASH'))).")
				ORDER BY e.timestamp";
			$messages = $synd_maindb->getAssoc($sql);

			foreach (array_keys($errors) as $key) {
				$errors[$key]['MESSAGE'] = $messages[$errors[$key]['HASH']];
				$this->_unwrapError($errors[$key]);
			}
		}
		
		return $errors;
	}

	function getError($hash) {
		global $synd_maindb;
		$sql = "
			SELECT e.* FROM synd_log e
			WHERE e.hash = ".$synd_maindb->quote($hash);
		return $this->_unwrapError($synd_maindb->getRow($sql));
	}

	function _unwrapError($error) {
		global $synd_maindb;
		if (!empty($error['REQUEST_DATA']))
			$error['REQUEST_DATA'] = unserialize($synd_maindb->unwrapBinary($error['REQUEST_DATA']));
		if (!empty($error['REQUEST_ENV']))
			$error['REQUEST_ENV'] = unserialize($synd_maindb->unwrapBinary($error['REQUEST_ENV']));
		return $error;
	}

	function getErrorCount() {
		global $synd_maindb;
		$sql = "SELECT COUNT(1) FROM synd_log e";
		return $synd_maindb->getOne($sql);
	}

	function _view_index(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();

		$this->_title = SyndLib::translate('Application and HTTP error log');
		SyndLib::attachHook('html_head_title', array($this, '_callback_html_head_title'));

		$page->assign('module', $this);
		$page->assign('request', $request);
		
		if (isset($request[0]) && null != ($error = $this->getError($request[0]))) {
			$page->assign('error', $error);
			$content = $page->fetch(tpl_design_path('module/log/error.tpl'));
		}
		else {
			$limit = 30;
			$offset = isset($request['offset']) ? $request['offset'] : 0;

			$page->assign('limit', $limit);
			$page->assign('offset', $offset);
			$page->assign('count', $this->getErrorCount());
			$page->assign('errors', $this->getErrors($offset, $limit, tpl_sort_order('log_exception')));

			$content = $page->fetch(tpl_design_path('module/log/index.tpl'));
		}
		
		return array('content' => $content);
	}
	
	function _callback_html_head_title(&$result) {
		return $result = $this->_title;
	}
	
	function _view_delete(Request $request, Template $page) {
		global $synd_maindb;
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();

		$sql = "
			DELETE FROM synd_log
			WHERE hash = ".$synd_maindb->quote($request[0]);
		$synd_maindb->query($sql);
	}

	function _view_clear(Request $request, Template $page) {
		global $synd_maindb;
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();
		$synd_maindb->truncate('synd_log');
	}

	/**
	 * @access	private
	 */
	function _trace($trace) {
		$tabStr = null;
		$maxLen = 120;
		$output = null;
		$trace  = array_reverse($trace);

		if (is_array($trace)) {
			foreach ($trace as $call) {
				$output .= $tabStr;
				$tabStr .= ' ';

				if (isset($call['class'])) 
					$output .= $call['class'].$call['type'];

				if (isset($call['args'])) {
					$args = $this->_formatArray($call['args'], $maxLen);
					$output .= $call['function'].'('.implode(', ', $args).')';
				}
				else
					$output .= $call['function'].'()';

				if (!isset($call['line']))
					$call['line'] = null;
				if (!isset($call['file']))
					$call['file'] = null;

				if (!empty($call['file']) || !empty($call['line']))
					$output .= " <span class=\"Info\">".str_replace($_SERVER['DOCUMENT_ROOT'], '', $call['file']).":{$call['line']}</span>";
				
				$output .= '<br />';
			}
		}

		return $output;
	}

	/**
	 * @access	private
	 */
	function _formatArray($arr, $maxlen = 100, $rec = 0) {
		$args = array();
		$i = 0;
		foreach(array_keys($arr) as $key) {
			if (is_null($arr[$key])) 
				$args[] = 'null';
			else if (is_array($arr[$key])) {
				if (0 == $rec)
					$args[] = 'array('.implode(', ',$this->_formatArray($arr[$key], $maxlen, $rec+1)).')';
				else
					$args[] = 'array['.count($arr[$key]).']';
			}
			else if (is_object($arr[$key])) {
				if (($arr[$key] instanceof DomainObject) && !empty($arr[$key]->nodeId))
					$args[] = $arr[$key]->nodeId;
				else if (($arr[$key] instanceof DomainObject) && null != $arr[$key]->nodeId())
					$args[] = $arr[$key]->nodeId();
				else
					$args[] = get_class($arr[$key]);
			}
			else if (is_bool($arr[$key]))
				$args[] = $arr[$key] ? 'true' : 'false';
			else if (is_numeric($arr[$key]))
				$args[] = $arr[$key];
			else {
				$arr[$key] = synd_htmlspecialchars(substr((string)$arr[$key], 0, $maxlen));
				if (strlen($arr[$key]) >= $maxlen) 
					$arr[$key] .= '..';
				$args[] = '"'.$arr[$key].'"';
			}

			$i++;
			if ($rec && $i >= 6) {
				$args[] = '...';
				break;
			}
		}
		return $args;
	}
	
	/**
	 * @access	private
	 */
	function _context($errcontext) {
		ob_start();
		print_r($errcontext);
		return substr(ob_get_clean(), 0, 1024*50);
	}
	
	/**
	 * @access	private
	 */
	function _formatExceptionType($errno) {
		switch ($errno) {
			case E_ERROR:
				return 'E_ERROR';
			case E_USER_ERROR:
				return 'E_USER_ERROR';
			case E_WARNING:
				return 'E_WARNING';
			case E_USER_WARNING:
				return 'E_USER_WARNING';
		}
		return '';
	}

	/**
	 * @access	private
	 */
	function _gc() {
		global $synd_maindb;
		try {
			$sql = "
				DELETE FROM synd_log
				WHERE timestamp < ".strtotime('-2 weeks');
			$synd_maindb->query($sql);
		}
		catch (DatabaseException $e) {}
	}
}
