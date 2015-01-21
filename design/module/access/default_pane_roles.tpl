<?
if (empty($request[1])) { 
	$access = Module::getInstance('access');
	$storage = SyndNodeLib::getDefaultStorage('issue');
	$database = $storage->getDatabase();

	$sql = "SELECT r.node_id FROM synd_access_role r";
	$roles = $storage->getInstances($database->getCol($sql));
	$roles = SyndLib::sort(SyndLib::filter($roles, 'isPermitted', 'read'));

	?>
	<dl>
	<? foreach (array_keys($roles) as $key) { ?>
		<dt><a href="<?= tpl_link('system','access','index','roles',$roles[$key]->nodeId) ?>"><?= $roles[$key]->toString() ?></a></dt>
		<dd><?= $roles[$key]->getDescription() ?></dd>
	<? } ?>
	</dl>

	<? if ($access->isPermitted('admin')) { ?>
	<ul class="Actions">
		<li><a href="<?= tpl_link_call('access','newRole') ?>"><?= tpl_text('Create new role') ?></a></li>
	</ul>
	<? } ?>
<? } else { ?>
	<? $this->render(SyndNodeLib::getInstance($request[1]), 'full_view.tpl') ?>
<? } ?>