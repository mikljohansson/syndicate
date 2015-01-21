<? if (count($files = $node->getFiles())) { ?>
	<table>
		<? foreach (array_keys($files) as $key) { ?>
		<tr>
			<td><a href="<?= $files[$key]->uri() ?>"><?= $files[$key]->toString() ?></a></td>
			<td class="Numeric" style="width:6em;"><?= tpl_text('%dKb',ceil($files[$key]->getSize()/1024)) ?></td>
			<td class="Numeric" style="width:10em;"><?= ucwords(tpl_strftime('%d %b %Y %R', $files[$key]->getCreated())) ?></td>
			<td style="padding-left:1em;"><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'unlink',$key) ?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= tpl_text('Delete') ?>" /></a></td>
		</tr>
		<? } ?>
	</table>
<? } else { ?>
<p><em><?= tpl_text('No attached files found') ?></em></p>
<? } ?>
<p>
	<input type="hidden" name="MAX_FILE_SIZE" value="20000000" />
	<input tabindex="<?= $tabindex++ ?>" type="file" name="data[file]" size="60" />
	<input tabindex="<?= $tabindex++ ?>" type="submit" value="<?= tpl_text('Attach') ?>" />
</p>