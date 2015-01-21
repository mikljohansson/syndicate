<? if (!isset($request[1])) { ?>
	<? 
	$access = Module::getInstance('access');
	$access->_title = tpl_text('Access control - Modules');
	
	$modules = (array)Module::runHook('manage_permissions');
	$modules = SyndLib::sort($modules);

	foreach (array_keys($modules) as $key) { ?>
		<div style="margin-bottom:0.5em;">
			<a href="<?= tpl_link('system','access','index','modules',$modules[$key]->classId()) ?>">
				<b><?= $modules[$key]->toString() ?></b></a>
			<div class="Abstract"><?= $modules[$key]->getDescription() ?></div>
		</div>
	<? } ?>
<? } else { ?>
	<? 
	$access = Module::getInstance('access');
	$access->_title = tpl_text('Access control - %s', ucfirst($request[1]));
	
	if (null !== ($module = Module::getInstance($request[1]))) { ?>
		<div class="Article">
			<div class="Header"><h2><?= $module->toString() ?></h2></div>
			<div class="Abstract"><?= $module->getDescription() ?></div>
		</div>
		<?= Module::runHook('permissions', $this, $module) ?>
	<? } else { ?>
		<div class="Warning"><h2><?= tpl_text("No module '%s' currently loaded", $request[1]) ?></h2></div>
	<? } ?>
<? } ?>