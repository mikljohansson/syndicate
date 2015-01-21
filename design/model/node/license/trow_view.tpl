	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<td><? 
			if ($node->isPermitted('write')) { 
				?><a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>"><img src="<?= tpl_design_uri('image/icon/record.gif') ?>" /></a><? 
			} else print '&nbsp;'; ?></td>
		<td style="white-space:nowrap;"><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>"><?= tpl_value($node->data['INFO_MAKE']) ?></a></td>
		<td><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>"><?= tpl_value($node->data['INFO_PRODUCT']) ?></a></td>
		<td><?= tpl_value($node->data['INFO_DESC']) ?></td>
		<td style="text-align:right;"><?= tpl_value($node->data['INFO_LICENSES']) ?></td>
	</tr>