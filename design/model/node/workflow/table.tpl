<table class="Enumeration">
	<thead>
		<tr>
			<th class="Actions">&nbsp;</th>
			<th><?= tpl_text('Name') ?></th>
			<th><?= tpl_text('Description') ?></th>
			<th><?= tpl_text('Project') ?></th>
		</tr>
	</thead>
	<tbody>
		<? foreach ($list as $node) { ?>
		<tr>
			<td>
				<? if ($node->isPermitted('write')) { ?>
				<a href="<?= tpl_link_call('issue','edit',$node->nodeId) ?>"><img src="<?= tpl_design_uri('image/icon/record.gif') ?>" alt="<?= tpl_text('Edit') ?>" /></a>
				<? } ?>
			</td>
			<td><a href="<?= tpl_link('issue','view',$node->nodeId) ?>"><?= $this->quote($node->toString()) ?></a></td>
			<td><?= $this->quote($node->data['INFO_DESC']) ?></td>
			<td><a href="<?= tpl_link('issue','view',$node->getParent()->nodeId,'admin','workflows') ?>"><?= $this->quote($node->getParent()->toString()) ?></a></td>
		</tr>
		<? } ?>
	</tbody>
</table>
