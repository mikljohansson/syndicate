	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<td><? 
			if ($node->isPermitted('write')) { 
				?><a href="<?= tpl_link_call('inventory','edit',$node->nodeId) ?>"><img src="<?= tpl_design_uri('image/icon/record.gif') ?>" /></a><? 
			} else print '&nbsp;'; ?></td>
		<td class="nowrap"><?= tpl_value($node->data['INFO_NUMBER']) ?></td>
		<td class="nowrap"><?= tpl_value($node->data['INFO_PROJECT_CODE']) ?></td>
		<td><?= tpl_value($node->data['INFO_HEAD']) ?></td>
		<td><?= tpl_value($node->data['INFO_DESC']) ?></td>
	</tr>