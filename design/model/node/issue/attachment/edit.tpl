<? if (count($files = array_merge($node->getFiles(),(array)$data['task']['DATA_FILES']))) { ?>
<table class="Attachments">
	<thead>
		<tr>
			<td colspan="3">&nbsp;</td>
			<td style="width:1%;">&nbsp;</td>
			<td style="width:1%;" class="Info" title="<?= tpl_text('Attach files to outgoing e-mail') ?>"><?= tpl_text('Attach') ?></td>
		</tr>
	</thead>
	<tbody>
		<? foreach ($files as $key => $file) { ?>
		<tr>
			<td style="width:1em;"><a href="<?= $file->uri() ?>"><img src="<?= 
				null != ($uri = tpl_design_uri('image/icon/16x16/'.strtolower(SyndLib::fileExtension($file->toString())).'.gif', false)) ? $uri : tpl_design_uri('image/icon/16x16/unknown.gif') 
				?>" alt="[<?= strtoupper(SyndLib::fileExtension($file->toString())) ?>]" width="16" height="16" /></a></td>
			<td><a href="<?= $file->uri() ?>"><?= $file->toString() ?></a></td>
			<td class="Numeric" style="width:6em;"><?= tpl_text('%dKb',ceil($file->getSize()/1024)) ?></td>
			<td style="padding-left:1em;"><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'unlink',$key) ?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= tpl_text('Delete') ?>" /></a></td>
			<td><?= tpl_form_checkbox('data[attachment][]',
				in_array($file->id(),(array)$request['data']['attachment']),
				$file->id(),"data[attachment][$key]") ?></td>
		</tr>
		<? } ?>
	</tbody>
</table>
<? } ?>
