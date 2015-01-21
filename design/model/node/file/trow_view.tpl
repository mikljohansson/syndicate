	<? $file = $node->getFile(); ?>
	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<td width="10">
			<? if ($node->isPermitted('write')) { ?>
			<a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) 
				?>"><img src="<?= tpl_design_uri('image/icon/record.gif') ?>" /></a>
			<? } else print '&nbsp;'; ?>
		</td>
		<td>
			<? if (!$file->isNull()) { ?>
			<a href="<?= $file->uri() ?>" title="<?= tpl_attribute($node->getDescription()) ?>"><?= $node->toString() ?></a>
			<? } else { ?>
			<?= $node->toString() ?>
			<? } ?>
		</td>
		<td><?= $node->getDescription() ?></td>
		<td class="Collapsed"><?= tpl_strftime('%Y-%m-%d %H:%M', $node->getCreated()) ?></td>
		<td class="Collapsed"><?= $file->getSize() ? round($file->getSize()/1024).'Kb' : '&nbsp;' ?></td>
	</tr>