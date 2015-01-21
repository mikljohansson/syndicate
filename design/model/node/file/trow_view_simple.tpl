	<? $file = $node->getFile(); ?>
	<tr>
		<td>
			<? if (!$file->isNull()) { ?>
			<a href="<?= $file->uri() ?>" title="<?= tpl_attribute($node->getDescription()) ?>"><?= $node->toString() ?></a>
			<? } else { ?>
			<?= $node->toString() ?>
			<? } ?>
		</td>
		<td><?= $node->getDescription() ?></td>
		<td><?= $file->getSize() ? round($file->getSize()/1024).'Kb' : '&nbsp;' ?></td>
		<td><?= tpl_strftime('%Y-%m-%d %H:%M', $node->getCreated()) ?></td>
		<td><? 
			if ($node->isPermitted('write')) { 
			?><a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) 
				?>"><img src="<?= tpl_design_uri('image/icon/edit.gif') ?>" alt="<?= tpl_text('Edit') ?>" title="<?= tpl_text('Edit this file') ?>" /></a><? 
			}
			if ($node->hasImage()) { 
			?><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) 
				?>"><img src="<?= tpl_design_uri('image/icon/image.gif') ?>" alt="<?= tpl_text('View') ?>" title="<?= tpl_text('View the file and thumbnail') ?>" /></a><? 
			} else print '&nbsp;';
			?></td>
	</tr>