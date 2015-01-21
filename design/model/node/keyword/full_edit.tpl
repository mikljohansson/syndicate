<div class="Article">
	<div class="RequiredField<?= isset($errors['INFO_HEAD'])?'InvalidField':'' ?>">
		<h3><?= tpl_text('Name') ?></h3>
		<input type="text" name="data[INFO_HEAD]" match="\w+" message="<?= tpl_text('Please provide a name') ?>" 
			value="<?= tpl_value($data['INFO_HEAD']) ?>" size="64" maxlength="255" />
	</div>
	<div class="RequiredField<?= isset($errors['PARENT_NODE_ID'])?'InvalidField':'' ?>">
		<h3><?= tpl_text('Parent') ?></h3>
		<select name="data[PARENT_NODE_ID]">
			<? $this->iterate(SyndLib::sort($node->getParentOptions()),'option_expand_keywords.tpl',array(
				'selected' => $node->_storage->getInstance($data['PARENT_NODE_ID']),
				'skip' => $node)) ?>
		</select>
	</div>
	<div class="RequiredField<?= isset($errors['INFO_DESC'])?'InvalidField':'' ?>">
		<h3><?= tpl_text('Description') ?></h3>
		<?= tpl_form_textarea('data[INFO_DESC]',$data['INFO_DESC'],array('cols'=>'48')) ?>
	</div>
	<div class="OptionalField">
		<?= tpl_form_checkbox('data[FLAG_MANDATORY]',$data['FLAG_MANDATORY']) ?>
			<label for="data[FLAG_MANDATORY]"><?= tpl_text('Category must be selected when creating issues') ?></label><br />
		<? /*
		<?= tpl_form_checkbox('data[FLAG_SINGLESELECT]',$data['FLAG_SINGLESELECT']) ?>
			<label for="data[FLAG_SINGLESELECT]"><?= tpl_text('A single subcategory may only be selected at a time') ?></label><br />
		*/ ?>
	</div>
	<span title="<?= tpl_text('Accesskey: %s','S') ?>">
		<input accesskey="s" type="submit" class="button" name="post" value="<?= tpl_text('Save') ?>" />
	</span>
	<input type="button" class="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
	<? if (!$node->isNew()) { ?>
	<input class="button" type="button" value="<?= tpl_text('Delete') ?>" onclick="window.location='<?= 
		tpl_view('node','delete',$node->nodeId) ?>';" />
	<? } ?>
</div>
