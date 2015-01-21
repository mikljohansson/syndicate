<h3><?= tpl_text('Subgroups') ?></h3>
<table class="list">
	<? foreach (array_keys($groups = $node->getGroups()) as $key) { ?>
	<tr>
		<td style="width:250px;">
			<a href="<?= tpl_link('course','clients',$groups[$key]->nodeId) ?>">
			<?= $groups[$key]->data['INFO_HEAD'] ?>
			</a>
		</td>
		<td><input type="checkbox" name="selection[]" value="<?= $clients[$key]->id() ?>" /></td>
	</tr>
	<? } ?>
</table>
<br />

<h3><?= tpl_text('Members in %s', $node->toString()) ?></h3>
<table class="list">
	<? foreach (array_keys($members = $node->getMembers()) as $key) { ?>
	<tr>
		<td style="width:250px;">
			<a href="<?= tpl_link('course','clients',$members[$key]->nodeId) ?>">
			<?= $members[$key]->toString() ?></a>
		</td>
		<td><input type="checkbox" name="selection[]" value="<?= $members[$key]->id() ?>" /></td>
	</tr>
	<? } ?>
</table>
