<div class="Article" style="width:600px;">
	<?= tpl_text('Title') ?>
	<div class="indent">
		<input type="text" name="data[INFO_HEAD]" value="<?= tpl_value($node->data['INFO_HEAD']) ?>" style="width:600px;" maxlength="255" />
	</div>

	<?= tpl_text('Short description') ?>
	<div class="indent">
		<input type="text" name="data[INFO_DESC]" value="<?= tpl_value($node->data['INFO_DESC']) ?>" style="width:600px;" maxlength="512" />
	</div>

	<br />
	<div class="indent">
		<input type="submit" class="button" name="post" value="<?= tpl_text('Save') ?>" />
		<input type="button" class="button" value="<?= tpl_text('Abort') ?>"  onclick="window.location='<?= tpl_uri_return() ?>';" />
		<? if (!$node->isNew()) { ?>
		<? $parent = $node->getParent(); ?> 
		<input class="button" type="button" value="<?= tpl_text('Delete') ?>" 
			onclick="window.location='<?= tpl_view_call($node->getHandler(),'delete',$node->nodeId,
				tpl_view($parent->getHandler(),'view',$parent->nodeId)) ?>';" />
		<? } ?>
	</div>
</div>