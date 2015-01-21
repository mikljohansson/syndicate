	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<td width="10">
			<? if ($node->isPermitted('write')) { ?>
			<a href="<?= tpl_link_call('inventory','edit',$node->nodeId) ?>"><img src="<?= tpl_design_uri('image/icon/record.gif') ?>" alt="" /></a>
			<? } else print '&nbsp;'; ?>
		<td>
			<? if (null != $node->data['INFO_SERIAL_INTERNAL']) { ?>
				<a href="<?= tpl_link('inventory','view',$node->nodeId) ?>">
				<?= $node->data['INFO_SERIAL_INTERNAL'] ?></a>
			<? } else print '&nbsp;'; ?>
		</td>
		<td><?= tpl_def($node->data['INFO_SERIAL_MAKER']) ?></td>
		<td class="nowrap">
			<a href="<?= tpl_link('inventory','view',$node->nodeId) ?>">
			<?= $node->getTitle() ?></a>
		</td>
		<td class="nowrap">
			<?= tpl_gui_node($node->getCustomer(),'head_view.tpl')  ?>
		</td>
		<td><?= tpl_gui_node($node->getFolder(),'head_view.tpl')  ?></td>
		<td class="OLE" onmouseover="this.parentNode.setAttribute('_checked',true);" onmouseout="this.parentNode.setAttribute('_checked',this.firstChild.checked);"><input type="checkbox" name="selection[]" value="<?= $node->id() ?>" /></td>
	</tr>	