	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<td width="10"><a href="<?= tpl_link_call('inventory','edit',$node->nodeId) ?>"><img src="<?= tpl_design_uri('image/icon/record.gif') ?>" border="0" /></a></td>
		<td>
			<a href="<?= tpl_link('inventory','view',$node->nodeId) ?>"><?= $node->toString() ?></a>
			<? if (null != ($contact = $node->getContact())) { ?>
			<span class="Info">(<?= $contact ?>)</span>
			<? } ?>
		</td>
		<td width="1%"><?= tpl_date('Y-m-d', $node->data['TS_CREATE']) ?></td>
		<td width="1%"><?= tpl_date('Y-m-d', $node->getExpire()) ?></td>
		<td width="1%"><?= tpl_date('Y-m-d', $node->getTerminatedTime()) ?></td>
		<td class="OLE" onmouseover="this.parentNode.setAttribute('_checked',true);" onmouseout="this.parentNode.setAttribute('_checked',this.firstChild.checked);"><input type="checkbox" name="selection[]" value="<?= $node->id() ?>" /></td>
	</tr>