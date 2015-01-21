<? $file = $node->getFile(); $image = $node->getImage(); ?>
<div class="Article">
	<input type="hidden" name="data[FLAG_PROMOTE]" value="<?= $data['FLAG_PROMOTE'] ?>" />
	<input type="hidden" name="MAX_FILE_SIZE" value="32000000" />

	<div class="RequiredField">
		<h4><?= tpl_text('Title') ?></h4>
		<input type="text" name="data[INFO_HEAD]" value="<?= tpl_value($data['INFO_HEAD']) ?>" size="43" maxlength="255" />
	</div>

	<div class="RequiredField">
		<h4><?= tpl_text('File') ?></h4><? if (isset($errors['DATA_FILE'])) { ?><span style="color:red;">*</span><? } ?>
		<input type="file" name="data[DATA_FILE]" size="43" />
		<? if (!$file->isNull()) { ?>
		<div class="Info"><?= tpl_text("Leave empty to keep the file <em>'%s'</em>.", $file->toString()) ?></div>
		<? } ?>
	</div>

	<div class="OptionalField">
		<h4><?= tpl_text('Thumbnail image') ?></h4>
		<input type="file" name="data[DATA_IMAGE]" size="43" />
		<div class="Info">
			<?= tpl_text('An optional thumbnail image that will be presented on pages.') ?>
			<? if (!$image->isNull()) { ?>
			<?= tpl_text("Leave empty to keep the image <em>'%s'</em>.", $image->toString()) ?>
			<?= tpl_translate('Click <a href="%s">here</a> to remove the image', tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'removeImage')) ?>
			<? } ?>
		</div>
	</div>

	<div class="OptionalField">
		<h4><?= tpl_text('Description') ?></h4>
		<?= tpl_form_textarea('data[INFO_DESC]', $data['INFO_DESC'], array('cols'=>60)) ?>
	</div>

	<input type="submit" name="post" value="<?= tpl_text('Save') ?>" />
	<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
	<? if (!$node->isNew()) { ?>
	<input type="button" value="<?= tpl_text('Delete') ?>" 
		onclick="window.location='<?= tpl_view_jump($node->getHandler(),'delete',$node->nodeId) ?>';" />
	<? } ?>

	<? include tpl_design_path('gui/errors.tpl'); ?>
</div>
