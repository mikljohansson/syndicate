<div class="Article">
	<div class="Header">
		<h1><?= $node->toString() ?></h1>
	</div>
	<?= SyndLib::runHook('inventory_installation_summary', $this, $node) ?>
	
	<h3><?= tpl_text('Files, receipts and invoices') ?></h3>
	<? if (count($files = $node->getFiles())) { ?>
	<table class="Files">
		<? foreach (array_keys($files) as $key) { ?>
		<tr>
			<td style="padding-right:1em;"><a href="<?= $files[$key]->uri() ?>"><?= basename($files[$key]->path()) ?></a></td>
			<td style="padding-right:1em;"><?= tpl_text('%dKb',$files[$key]->getSize()/1024) ?></td>
			<td style="padding-right:1em;"><?= ucwords(tpl_strftime('%A, %d %b %Y %R', $files[$key]->getCreated())) ?></td>
			<td><a href="<?= tpl_link_call('inventory','invoke',$node->nodeId,'fileDelete',array('file'=>$key)) ?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= tpl_text('Delete') ?>" /></a></td>
		</tr>
		<? } ?>
	</table>
	<br />
	<? } ?>
	<form action="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'fileUpload') ?>" method="post" enctype="multipart/form-data">
		<input type="hidden" name="MAX_FILE_SIZE" value="5000000" />
		<input type="file" name="file" size="73" />
		<input type="submit" value="<?= tpl_text('Upload') ?>" />		
	</form>
	
	<h3><?= tpl_text('Items') ?></h3>
	<?= tpl_gui_table('item',$node->getItems(),'view.tpl') ?>
</div>