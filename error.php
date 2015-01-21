<?php
require_once 'synd.inc';
require_once 'core/lib/Template.class.inc';

$status = $_SERVER['REDIRECT_STATUS']; 
$uri = $_SERVER['REDIRECT_URL'];
Module::runHook('http_error', $status, $uri);

$page = new Template();
$page->assign('status', $status);
$page->assign('uri', $uri);

if (null != ($template = tpl_design_path("gui/error/{$status}.tpl", false)))
	$page->assign('content', $page->fetch($template));
else 
	$page->assign('content', $page->fetch(tpl_design_path('gui/error/unknown.tpl')));

$page->display(tpl_design_path('page.tpl'));
