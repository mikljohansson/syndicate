<table class="Items" width="100%" cellpadding="2">
	<thead>
		<tr class="<?= $this->cycle(array('odd','even')) ?>">
			<th width="10">&nbsp;</th>
			<th><?= tpl_text('Model') ?></th>
			<th><?= tpl_text('Serial') ?></th>
			<th><?= tpl_text('Handed out') ?></th>
			<th style="width:8em;"><?= tpl_text('Returned') ?></th>
			<th style="width:8em;"><?= tpl_text('Folder') ?></th>
			<? if (empty($hideCheckbox)) { ?>
			<th class="OLE">
				<? if (isset($collection)) { ?>
				<?= tpl_form_checkbox('collections[]',false,$collection->id()) ?>
				<? } else print '&nbsp;'; ?>
			</th>
			<? } ?>
		</tr>
	</thead>
	<tbody>
		<? foreach ($list as $node) { 
			$item = $node->getItem(); ?>
		<tr class="<?= $this->cycle() ?>">
			<td width="10">
				<? if ($item->isPermitted('write')) { ?>
				<a href="<?= tpl_link_call('inventory','edit',$item->nodeId) 
					?>"><img src="<?= tpl_design_uri('image/icon/record.gif') ?>" /></a>
				<? } else print '&nbsp;'; ?>
			</td>
			<td class="nowrap">
				<a href="<?= tpl_link('inventory','view',$item->nodeId) ?>"><?= tpl_default($item->getTitle(), tpl_text('Unknown')) ?></a>
			</td>
			<td><a href="<?= tpl_link('inventory','view',$item->nodeId) ?>"><?= $item->data['INFO_SERIAL_MAKER'] ?></a></td>
			<td><?= tpl_text('%s by %s', tpl_strftime('%Y-%m-%d', $node->data['TS_CREATE']), $this->fetchnode($node->getCreator(),'contact.tpl')) ?></td>
			<td><?= $node->data['TS_EXPIRE'] ? tpl_strftime('%Y-%m-%d', $node->data['TS_EXPIRE']) : '&nbsp;' ?></td>
			<td><? $this->render($item->getParent(),'head_view.tpl') ?></td>
			<? if (empty($hideCheckbox)) { ?>
			<td class="OLE" onmouseover="this.parentNode.setAttribute('_checked',true);" onmouseout="this.parentNode.setAttribute('_checked',this.firstChild.checked);"><input type="checkbox" name="selection[]" value="<?= $item->id() ?>" /></td>
			<? } ?>
		</tr>
		<? } ?>
	</tbody>
</table>	