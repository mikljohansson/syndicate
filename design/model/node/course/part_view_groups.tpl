<h3><?= tpl_text('Subgroups of %s', $group->toString()) ?></h3>
<? if (count($groups = $group->getGroups())) { ?>
	<? $groups = SyndLib::sort($groups); ?>
	<table class="indent">
		<? foreach (array_keys($groups) as $key) { ?>
		<tr>
			<td style="width:250px;">
				<a href="<?= tpl_link($node->getHandler(),'view',
					$node->nodeId,'admin','groups',$groups[$key]->nodeId) ?>">
				<?= $groups[$key]->toString() ?></a>
			</td>
			<td>
				<input type="checkbox" name="selection[]" value="<?= $groups[$key]->id() ?>" />
			</td>
		</tr>
		<? } ?>
	</table>
<? } else { ?>
	<div class="indent">
		<em><?= tpl_text('No groups found') ?></em>
	</div>
<? } ?>

<h3><?= tpl_text('Members of %s', $group->toString()) ?></h3>
<? if (count($relations = $group->getMemberRelations())) { ?>
	<? $relations = SyndLib::sort($relations); ?>
	<table class="indent">
		<thead>
			<tr>
				<th><?= tpl_text('Name') ?></th>
				<th>&nbsp;</th>
				<th style="padding-right:1em;"><?= tpl_text('Registered') ?></th>
				<th colspan="2"><?= tpl_text('Lowest score') ?></th>
			</tr>
		</thead>
		<tbody>
			<? foreach (array_keys($relations) as $key) { ?>
			<tr>
				<td style="width:250px;">
					<? $client = $relations[$key]->getChild(); ?>
					<a href="<?= tpl_link($node->getHandler(),'view',
						$node->nodeId,'admin','groups',$group->nodeId,$client->nodeId) ?>">
					<?= $client->toString() ?></a>
				</td>
				<td><input type="checkbox" name="selection[]" value="<?= $relations[$key]->id() ?>" /></td>
				<td><?= date('Y-m-d', $relations[$key]->data['TS_CREATE']) ?></td>
				<? if (null != ($attempt = $node->getWorstAttempt($client))) { ?>
				<td><? $this->render($attempt,'head_view_progress.tpl') ?></td>
				<td><? $this->render($attempt,'head_view.tpl') ?></td>
				<? } else { ?>
				<td>&nbsp;</td>
				<td>&nbsp;</td>
				<? } ?>
			</tr>
			<? } ?>
		</tbody>
	</table>
<? } else { ?>
	<div class="indent">
		<em><?= tpl_text('No clients found') ?></em>
	</div>
<? } ?>


<? if ($group->isPermitted('write')) { ?>
<ul class="Actions" style="margin-top:10px;">
	<li><a href="<?= tpl_link_call($node->getHandler(),'view',
		$node->nodeId,'admin','groups',$group->nodeId,'email') ?>">
		<?= tpl_text('Email this group') ?></a></li>
	<li><a href="javascript:synd_ole_call('<?= tpl_view_call('course','cut',
		isset($group)?$group->nodeId:'case.course') ?>')" 
		title="<?= tpl_text('Cut the selected groups and/or users from this group') 
		?>"><?= tpl_text('Cut selected') ?></a></li>
	<li><a href="javascript:synd_ole_call('<?= tpl_view_call('course','delete') ?>')" 
		title="<?= tpl_text('Delete the selected groups and/or users') 
		?>"><?= tpl_text('Delete selected') ?></a></li>
	<li><a href="<?= tpl_link_call($node->getHandler(),'view',
		$node->nodeId,'admin','groups',$group->nodeId,'create_group') ?>">
		<?= tpl_text('Create new subgroup') ?></a></li>
	<li><a href="<?= tpl_link_call($node->getHandler(),'view',
		$node->nodeId,'admin','groups',$group->nodeId,'create_clients') ?>">
		<?= tpl_text('Create new client accounts') ?></a></li>
</ul>
<? } ?>