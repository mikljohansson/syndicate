<?php
/**
 * Add something like this to your crontab, or drop a shell script 
 * in your /etc/cron.daily/ directory (if you have one) that does 
 * the same.
 *
 *  0 4 * * *	/usr/bin/php /var/www/html/synd/cron.php
 *
 * Users or the Oracle database should take care to setup the 
 * environment variables it depends on before executing this script. 
 * See docs/oracle.txt file for more Oracle specific information.
 */

if (isset($_SERVER['REMOTE_ADDR']))
	die('This script must be run from commandline.');

error_reporting(E_ALL);
require_once 'synd.inc';

if (!ini_get("safe_mode"))
	set_time_limit(3600);

Module::runHook('cron');
