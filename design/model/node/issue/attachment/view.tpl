<? if (count($files = $node->getFiles())) { ?>
<table class="Attachments">
	<thead>
		<tr>
			<td colspan="2"><h5><?= tpl_text('Attachments') ?></h5></td>
			<td style="width:1%;">&nbsp;</td>
		</tr>
	</thead>
	<tbody>
		<? foreach (array_keys($files) as $key) { ?>
		<tr>
			<td style="width:1em;"><a href="<?= $files[$key]->uri() ?>"><img src="<?= 
				null != ($uri = tpl_design_uri('image/icon/16x16/'.strtolower(SyndLib::fileExtension($files[$key]->toString())).'.gif', false)) ? $uri : tpl_design_uri('image/icon/16x16/unknown.gif') 
				?>" alt="[<?= strtoupper(SyndLib::fileExtension($files[$key]->toString())) ?>]" width="16" height="16" /></a></td>
			<td><a href="<?= $files[$key]->uri() ?>"><?= $files[$key]->toString() ?></a></td>
			<td class="Numeric" style="width:6em;"><?= tpl_text('%dKb',ceil($files[$key]->getSize()/1024)) ?></td>
		</tr>
		<? } ?>
	</tbody>
</table>
<? } ?>
