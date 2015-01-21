<? tpl_load_script(tpl_design_path('js/form.js')) ?>
<div class="Article">
	<div class="Section Required">
		<div class="Caption"><?= tpl_text('Name') ?></div>
		<input type="text" name="data[INFO_HEAD]" value="<?= tpl_attribute($data['INFO_HEAD']) ?>" 
			match=".+" message="<?= tpl_text('Please provide a name') ?>" maxlength="512" />
	</div>
	<div class="Section">
		<div class="Caption"><?= tpl_text('Description') ?></div>
		<input type="text" name="data[INFO_DESC]" value="<?= tpl_attribute($data['INFO_DESC']) ?>" 
			maxlength="2000" />
		<div class="Help"><?= tpl_text('Short description of role') ?></div>
	</div>

	<input type="submit" name="post" value="<?= tpl_text('Save') ?>" />
	<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
</div>