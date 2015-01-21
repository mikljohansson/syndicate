<input type="hidden" name="stack[]" value="/" />
<div class="FlowingForm">
	<fieldset>
		<legend><?= $this->text('Workflow') ?></legend>
		<div class="Required<?= isset($errors['INFO_HEAD'])?' Invalid':'' ?>">
			<label for="data[INFO_HEAD]"><?= $this->text('Name') ?></label>
			<?= tpl_form_text('data[INFO_HEAD]', $data['INFO_HEAD']) ?>
		</div>
		<div class="Required<?= isset($errors['INFO_EMAIL'])?' Invalid':'' ?>">
			<label for="data[INFO_EMAIL]"><?= $this->text('E-mail') ?></label>
			<?= tpl_form_text('data[INFO_EMAIL]', $data['INFO_EMAIL']) ?>
		</div>
		<div class="Optional<?= isset($errors['INFO_PHOTO'])?' Invalid':'' ?>">
			<label for="data[INFO_PHOTO]"><?= $this->text('Photo') ?></label>
			<input type="hidden" name="MAX_FILE_SIZE" value="2000000" />
			<input type="file" name="data[INFO_PHOTO]" />
		</div>
		<div class="Optional<?= isset($errors['INFO_DESC'])?' Invalid':'' ?>">
			<label for="data[INFO_DESC]"><?= $this->text('Description') ?></label>
			<textarea name="data[INFO_DESC]" id="data[INFO_DESC]"><?= $this->quote($data['INFO_DESC']) ?></textarea>
		</div>
	</fieldset>

	<p>
		<input accesskey="s" title="<?= $this->text('Keyboard shortcut: Alt+%s','S') ?>" type="submit" name="post" value="<?= $this->text('Save') ?>" />
		<input accesskey="a" title="<?= $this->text('Keyboard shortcut: Alt+%s','A') ?>" type="button" value="<?= $this->text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
		<? if (!$node->isNew()) { ?>
		<input type="button" value="<?= $this->text('Delete') ?>" onclick="window.location='<?= tpl_link_jump('issue','delete',$node->nodeId) ?>';" />
		<? } ?>
	</p>
</div>
