<?php
/**
 * Include this file to enable the stacktrace/context/debug error handler.
 * @package		synd.core
 */

global $synd_error_handler;
$synd_error_handler = true;

error_reporting(E_ALL | E_STRICT);
set_error_handler('_synd_callback_error');

assert_options(ASSERT_BAIL, false);
assert_options(ASSERT_ACTIVE, true);
assert_options(ASSERT_WARNING, false);
assert_options(ASSERT_CALLBACK, '_synd_callback_assert');

/**
 * Error callback. Pretty prints the error stacktrace and context.
 *
 * Displays the regular message and options for showing the stacktrace
 * and variable context.
 *
 * @access	private
 */
function _synd_callback_error($errno, $errstr, $errfile, $errline, $errcontext) {
	static $errorCount = 0;
	if (0 == (error_reporting() & $errno) || $errno == E_NOTICE && ('tpl' == SyndLib::fileExtension($errfile))) 
		return;

	global $synd_error_triggered;
	$synd_error_triggered = true;
	
	if (75 == ++$errorCount)
		print 'Too many errors';
	if ($errorCount >= 75)
		return;

	// Log to error_log
	if (ini_get('log_errors')) {
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

			case E_STRICT:
				$logline = "Strict: ";
				break;

			case E_RECOVERABLE_ERROR:
				$logline = "Recoverable: ";
				break;
			
			default:
				$logline = "Unknown ($errno): ";
				break;
		}

		$logline = "$errstr in $errfile on line $errline";
		$logline = preg_replace('/\s+/', ' ', $logline);

		error_log($logline);
	}

	// Format the backtrace
	$trace = debug_backtrace();
	array_shift($trace);
	$traceOutput = _error_format_trace($trace);
	
	// Clone context symbolspace and print
	$context = array_slice($errcontext,0);

	unset($context['GLOBALS']);
	unset($context['synd_config']);

	ob_start();
	print_r($context);
	$contextOutput = '<pre>'.htmlspecialchars(ob_get_clean()).'</pre>';
	
	// File the error occurred in
	$synd_dir = dirname(dirname(__FILE__));
	$errfile  = str_replace($synd_dir, '', $errfile);
	
	// Determine function the error occurred in
	$index = 0;
	$function = null;
	
	if (count($trace) > 1 && 'debug' == $trace[1]['function']) {
		$errfile = $trace[1]['file'];
		$errline = $trace[1]['line'];
		$index++;
	}
	
	if (count($trace) && 'unknown' != $trace[$index]['function']) {
		$function = $trace[$index]['function']."()";
		if (isset($trace[$index]['class']))
			$function = $trace[$index]['class'].$trace[$index]['type'].$function;
	}

	print "<!-- \"> -->\n<div style=\"white-space:nowrap\">";

	switch ($errno) {
		case E_ERROR:
		case E_USER_ERROR:
			print "<b>Error:</b> $function $errstr";
			break;

		case E_WARNING:
		case E_USER_WARNING:
			print "<b>Warning:</b> $function $errstr";
			break;

		case E_NOTICE:
		case E_USER_NOTICE:
			print "<b>Notice:</b> $function $errstr";
			break;

		case E_STRICT:
			print "<b>Strict</b>: $function $errstr";
			break;

		case E_RECOVERABLE_ERROR:
			print "<b>Recoverable</b>: $function $errstr";
			break;
		
		default:
			print "<b>Unknown ($errno):</b> $function $errstr";
			break;
	}

	print " in $errfile on line $errline<br />\n";
	if (!empty($trace))
		_error_print_layer('stacktrace', $traceOutput);
	if (!empty($context))
		_error_print_layer('context', $contextOutput);
	print "</div>\n\n";

	if (E_ERROR == $errno || E_USER_ERROR == $errno) {
		if (!headers_sent() && isset($_SERVER['REMOTE_ADDR']))
			header('HTTP/1.0 500 error: Internal Server Error');
		exit(1);
	}
}

/**
 * Assert callback. Wraps the regular error callback.
 * @access	private
 */
function _synd_callback_assert($errfile, $errline, $errstr) {
	_synd_callback_error(E_USER_WARNING, "Assertion $errstr failed", $errfile, $errline, array());
}

/**
 * @access	private
 */
function _error_print_js() {
	?>
	<script type="text/javascript">
	<!-- "> -->
	<!--
	function _error_toggle_layer(sId) {
		if (undefined != document.selection && document.selection.createRange().text.length) 
			return;
		var oLayer = document.getElementById(sId);
		if (null != oLayer && undefined != oLayer.style)
			oLayer.style.visibility = ('visible' == oLayer.style.visibility) ? 'hidden' : 'visible';
	}
	//-->
	</script>
	<?
}

/**
 * @access	private
 */
function _error_print_layer($name, $content) {
	static $count = 0;
	if (0 == $count) {
		SyndLib::attachHook('shutdown', '_error_print_js');
		_error_print_js();
	}

	$count++;
	$layer = "_error_layer_$count";

	?>
	<a href="javascript:_error_toggle_layer('<?= $layer ?>');"><?= $name ?></a>
	<div id="_error_layer_<?= $count ?>" onclick="_error_toggle_layer('<?= $layer ?>');" 
		style="font-size:12px; position:absolute; z-index:1; visibility:hidden; background-color:white; border:1px solid red; padding:5px">
		<?= $content ?> 
	</div>
	<?
}

/**
 * @access	private
 */
function _error_format_array($arr, $maxlen = 100, $rec = 0) {
	$args = array();
	$i = 0;
	foreach(array_keys($arr) as $key) {
		if (is_null($arr[$key])) 
			$args[] = 'null';
		else if (is_array($arr[$key])) {
			if (0 == $rec)
				$args[] = 'array('.implode(', ',_error_format_array($arr[$key], $maxlen, $rec+1)).')';
			else
				$args[] = 'array['.count($arr[$key]).']';
		}
		else if (is_object($arr[$key])) {
			if (($arr[$key] instanceof DomainObject) && !empty($arr[$key]->nodeId))
				$args[] = $arr[$key]->nodeId;
			else
				$args[] = get_class($arr[$key]);
		}
		else if (is_bool($arr[$key]))
			$args[] = $arr[$key] ? 'true' : 'false';
		else if (is_numeric($arr[$key]))
			$args[] = $arr[$key];
		else {
			$arr[$key] = htmlspecialchars(substr($arr[$key], 0, $maxlen));
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
function _error_format_trace($trace) {
	$maxLen = 120;
	$output = "<div style=\"font-family:courier new;text-align:left;\"><pre>\n";
	$trace  = array_reverse($trace);
	
	foreach ($trace as $i => $call) {
		$output .= "#$i ";
		if (isset($call['class'])) 
			$output .= $call['class'].$call['type'];

		if (isset($call['args'])) {
			$args = _error_format_array($call['args'], $maxLen);
			$output .= $call['function'].'('.implode(', ', $args).')';
		}
		else
			$output .= $call['function'].'()';

		if (!isset($call['line']))
			$call['line'] = null;
		if (!isset($call['file']))
			$call['file'] = null;
		$call['file'] = str_replace($_SERVER['DOCUMENT_ROOT'], '', $call['file']);
			
		$output .= '<span style="color:#808080;font-size:0.9em"> # line '.$call['line'].', file: '.$call['file']."</a></span>\n";
	}
	
	$output .= '</pre></div>';
	return $output;
}

