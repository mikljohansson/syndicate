<?php
set_include_path(get_include_path().':'.dirname(dirname(dirname(dirname(__FILE__)))).'/local'.':'.dirname(dirname(dirname(__FILE__))));
require_once 'synd.inc';

$module = Module::getInstance('issue');
$module->runCleanup();
//system('killall php');
