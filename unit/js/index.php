<?
function _page($page) {
	return rawurlencode('http://'.$_SERVER['SERVER_NAME'].dirname($_SERVER['PHP_SELF']).'/'.$page);
}
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
	<title>Synd JavaScript unit tests</title>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
	<link rel="stylesheet" type="text/css" href="jsunit/css/jsUnitStyle.css">
</head>
<body>
	<dl>
		<dt><a href="jsunit/testRunner.html?testpage=<?= _page('xmlrpc.html') ?>">XML-RPC</a></dt>
		<dd>Unit tests for the JavaScript XML-RPC implementation</dd>
		<dt><a href="jsunit/testRunner.html?testpage=<?= _page('http.html') ?>">HTTP connection</a></dt>
		<dd>Unit tests for the JavaScript HTTP connection implementation</dd>
	</dl>
</body>