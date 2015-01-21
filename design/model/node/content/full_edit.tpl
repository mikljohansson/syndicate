<div class="Article">
	<div class="RequiredField">
		<h3><?= tpl_text('Title') ?></h3>
		<input type="text" name="data[INFO_HEAD]" value="<?= tpl_value($data['INFO_HEAD']) ?>" size="65" />
	</div>
	<div class="OptionalField">
		<h3><?= tpl_text('Subtitle') ?></h3>
		<input type="text" name="data[INFO_DESC]" value="<?= tpl_value($data['INFO_DESC']) ?>" size="65" />
	</div>

	<input type="submit" class="button" name="post" value="<?= tpl_text('Save') ?>" />
	<input type="button" class="button" value="<?= tpl_text('Abort') ?>" 
		onclick="window.location='<?= tpl_view($node->getHandler(),'view',$node->nodeId) ?>';" />
	<? if (!$node->isNew()) { ?>
	<? $parent = $node->getParent(); ?> 
	<input class="button" type="button" value="<?= tpl_text('Delete') ?>" 
		onclick="window.location='<?= tpl_view_call($node->getHandler(),'delete',$node->nodeId,
			tpl_view($parent->getHandler(),'view',$parent->nodeId)) ?>';" />
	<? } ?>
</div>