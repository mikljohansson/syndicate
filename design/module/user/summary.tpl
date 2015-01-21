<h1><?= $user->toString() ?></h1>
<? if (tpl_gui_path(get_class($user),'summary.tpl',false)) { ?>
	<? $this->render($user,'summary.tpl') ?>
<? } else { ?>
	<?
	$branch = array();
	for ($parent = $user->getOrganization(); !$parent->isNull(); $parent = $parent->getParent())
		$branch[] = $parent; 
	?>
	<table class="Vertical Block">
		<tbody>
			<tr class="<?= tpl_cycle(array('odd','even')) ?>">
				<th><?= tpl_text('Username') ?></th>
				<td style="width:40%;"><?= tpl_def($user->getLogin()) ?></td>
				<th><?= tpl_text('Roles') ?></th>
				<td style="width:40%;">
					<? foreach (array_keys($roles = SyndLib::sort($user->getRoles())) as $key) { 
						if ($i++) print ', '; 
						print $this->fetchnode($roles[$key],'head_view.tpl');
					} ?>
				</td>
			</tr>
			<tr class="<?= tpl_cycle() ?>">
				<th><?= tpl_text('E-mail') ?></th>
				<td><?= tpl_email($user->getEmail()) ?></td>
				<th rowspan="2"><?= tpl_text('Organization') ?></th>
				<td rowspan="2">
					<? if (!empty($branch)) { ?>
					<ul class="Enumeration">
						<? foreach (array_reverse(array_keys($branch)) as $key) { ?>
						<li><a href="<?= tpl_link('user','summary',$branch[$key]->nodeId) ?>"><?= $branch[$key]->toString() ?></a></li>
						<? } ?>
					</ul>
					<? } ?>
				</td>
			</tr>
			<tr class="<?= tpl_cycle() ?>">
				<th><?= tpl_text('Phone') ?></th>
				<td><?= tpl_def($user->getPhone()) ?></td>
			</tr>
		</tbody>
	</table>
<? } ?>
<?= SyndLib::runHook('user_summary_default', $request, $this, $user) ?>