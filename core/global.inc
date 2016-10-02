<?php
require_once 'core/i18n.inc';

// Include debugging support
if (isset($_SERVER['REMOTE_ADDR'], $synd_config['debug_allowed_ips'][$_SERVER['REMOTE_ADDR']]))
	require_once 'core/debug.inc';

require_once 'core/lib/SyndLib.class.inc';
if (isset($_REQUEST['profile']))
	profiler_start();
profiler_enter('global setup');

global $synd_config, $synd_maindb, $synd_user;
require_once 'core/Module.class.inc';
require_once 'core/db/Database.class.inc';
require_once 'core/model/DomainObject.class.inc';
require_once 'core/model/SyndType.class.inc';
require_once 'core/lib/SyndNodeLib.class.inc';
require_once 'core/lib/TemplateLib.inc';

/**
 * @access	private
 */
function _synd_callback_shutdown() {
	SyndLib::runHook('shutdown');
}
register_shutdown_function('_synd_callback_shutdown');

/**
 * Main database connection
 * @global	object	Database	$GLOBALS['synd_maindb']
 * @name	$synd_maindb
 */
$synd_maindb = DatabaseManager::getConnection($synd_config['db']['main']);

// Load modules
foreach ($synd_config['modules'] as $__id) {
	if (null === Module::getInstance($__id))
		trigger_error("Unknown module '$__id'", E_USER_ERROR);
}

// Null object pattern for $synd_user
if (!isset($synd_user))
	$synd_user = SyndNodeLib::getInstance('user_null.null');
	
profiler_leave('global setup');
