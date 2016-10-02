		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td width="10">
				<? if ($node->isPermitted('write')) { ?>
				<a href="<?= tpl_link_call('inventory','edit',$node->nodeId) 
					?>"><img src="<?= tpl_design_uri('image/icon/record.gif') ?>" /></a>
				<? } else print '&nbsp;'; ?>
			</td>
			<td><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>"><?= $node->toString() ?></a></td>
			<td><?= synd_htmlspecialchars(tpl_chop($node->getDescription(),50)) ?></td>
			<td class="OLE" onmouseover="this.parentNode.setAttribute('_checked',true);" onmouseout="this.parentNode.setAttribute('_checked',this.firstChild.checked);"><input type="checkbox" name="selection[]" value="<?= $node->id() ?>" /></td>
		</tr>