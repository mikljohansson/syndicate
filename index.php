<?php
require_once 'core/Controller.class.inc';
require_once 'core/Module.class.inc';
require_once 'core/lib/SyndLib.class.inc';
require_once 'core/lib/Template.class.inc';

try {
	require_once 'synd.inc';
	if (!isset($_REQUEST['sq']))
		throw new InvalidArgumentException("Request parameter 'sq' must be set");

	$uribase = isset($_SERVER['REDIRECT_URL']) ? 
		substr($_SERVER['REDIRECT_URL'], 0, strlen($_SERVER['REDIRECT_URL']) - strlen($_REQUEST['sq'])) : 
		$_SERVER['SCRIPT_NAME'].'?sq=';
	$processor = new RequestProcessor();

	// Execute multiplexed views
	if (isset($_REQUEST['mplex'])) {
		if (isset($_FILES['mplex'])) {
			require_once 'core/lib/SyndHTTP.class.inc';
			$files = SyndLib::filesTransform($_FILES);
		}

		foreach (array_keys($_REQUEST['mplex']) as $context) {
			$request = new HttpRequest(
				$_SESSION, $uribase, $context, $_REQUEST['mplex'][$context], 
				isset($_GET['mplex'][$context]) ? $_GET['mplex'][$context] : array(), 
				isset($_POST['mplex'][$context]) ? $_POST['mplex'][$context] : array(), 
				isset($files[$context]) ? $files[$context] : array(), $_COOKIE);
			$processor->request($request);
		}
	}

	// Execute primary view
	$context = $_REQUEST['sq'];
	unset($_REQUEST['sq'], $_GET['sq']);	
	
	$request = new HttpRequest(
		$_SESSION, $uribase, $context, $_REQUEST, $_GET, $_POST, 
		SyndLib::filesTransform($_FILES), $_COOKIE);
	if (false === ($response = $processor->request($request)))
		throw new NotFoundException();

	// Run shutdown hook early
	SyndLib::runHook('shutdown');
	
	// Display page or perform redirection
	global $synd_error_triggered;
	if (isset($response['content']) || isset($synd_error_triggered)) {
		if (!isset($response['content']))
			$response['content'] = '';

		if (is_array($response) && array_key_exists('page', $response) && null == $response['page'])
			print $response['content'];
		else { 
			$page = new Template(array_reverse($synd_config['dirs']['design']), $request);
			$page->assign('path', $context);
			$page->assign('content', $response['content']);
			$page->display(tpl_design_path(isset($response['page']) ? $response['page'] : 'page.tpl'));
		}
	}
	else if (!headers_sent()) {
		if (isset($response['redirect']) && '' != $response['redirect'])
			header('Location: '.$response['redirect']);
		else if (isset($request['stack']))
			header('Location: '.tpl_uri_return());
	}
}
catch (InvalidArgumentException $e)	{_synd_display_error($request, 400, $e);}
catch (AuthorizationException $e)	{_synd_display_error($request, 401, $e, $e->getMessage());}
catch (ForbiddenException $e)		{_synd_display_error($request, 403, $e, $e->getMessage());}
catch (NotFoundException $e)		{_synd_display_error($request, 404, $e, $e->getMessage());}
catch (Exception $e)				{_synd_display_error($request, 500, $e);}

/**
 * @access	private
 */
function _synd_display_error(Request $request, $status, Exception $exception, $message = null) {
	global $synd_config;
	Module::runHook('http_error', $status, $_SERVER['REQUEST_URI'], $exception);
	
	$page = new Template(array_reverse($synd_config['dirs']['design']), $request);
	$page->assign('uri', $_SERVER['REQUEST_URI']);
	if (null != $message)
		$page->assign('message', $message);

	if (isset($_SERVER['REMOTE_ADDR'], $synd_config['debug_allowed_ips'][$_SERVER['REMOTE_ADDR']]))
		$page->assign('exception', $exception);
	
	$page->assign('content', $page->fetch(tpl_design_path("gui/error/$status.tpl")));

	if (!isset($_SERVER['REMOTE_ADDR'], $synd_config['debug_allowed_ips'][$_SERVER['REMOTE_ADDR']])) {
		for ($i=ob_get_level(); $i > 0; $i--)
			ob_end_clean();
	}
		
	$page->display(tpl_design_path('page.tpl'));
}
