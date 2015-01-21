<?php

$host = isset($_SERVER['HTTP_HOST']) ? $_SERVER['HTTP_HOST'] : $_SERVER['SERVER_NAME'];
if (false !== ($i = strpos($host, ':')))
	$host = substr($host, 0, $i);

if (isset($_SERVER['HTTPS'])) {
	$uri = 'https://'.$host;
	if (443 != $_SERVER['SERVER_PORT'])
		$uri .= ':'.$_SERVER['SERVER_PORT'];
}
else {
	$uri = 'http://'.$host;
	if (80 != $_SERVER['SERVER_PORT'])
		$uri .= ':'.$_SERVER['SERVER_PORT'];
}

header("Location: $uri".substr(dirname(__FILE__), strlen(rtrim($_SERVER['DOCUMENT_ROOT'],DIRECTORY_SEPARATOR))).'/httpconnection-001-head.txt');
