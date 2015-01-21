<? tpl_load_script(tpl_design_uri('js/form.js')) ?>
<div class="Article" style="width:600px;">
	<? include tpl_design_path('gui/errors.tpl'); ?>
	
	<div class="RequiredField<? if(isset($errors['INFO_HEAD'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Name') ?></h3>
		<input type="text" name="data[INFO_HEAD]" match="\w+" message="<?= tpl_text('Please provide a name') ?>" 
			value="<?= tpl_value($data['INFO_HEAD']) ?>" size="64" maxlength="255" />
	</div>

	<div class="RequiredField<? if(isset($errors['INFO_DESC'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Description') ?></h3>
		<?= tpl_form_textarea('data[INFO_DESC]',$data['INFO_DESC'],
			array('cols'=>'48','match'=>'\w+','message'=>tpl_text('Please provide a description'))) ?>
	</div>
	
	<div class="RequiredField<? if(isset($errors['PARENT_NODE_ID'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Parent folder') ?></h3>
		<select name="data[PARENT_NODE_ID]">
			<?$this->display('module/inventory/folder_options.tpl',array('selected' => SyndNodeLib::getInstance($data['PARENT_NODE_ID'])));?>
		</select>
	</div>
	
	<div class="OptionalField<? if(isset($errors['INFO_NUMBER'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text("Costcenter number") ?></h3>
		<input type="text" name="data[INFO_NUMBER]" value="<?= tpl_value($data['INFO_NUMBER']) ?>" size="64" maxlength="64" />
		<div class="Info"><?= tpl_text('The number or code identifying this cost center in economic systems') ?></div>
	</div>
	<div class="OptionalField<? if(isset($errors['INFO_PROJECT_CODE'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text("Project code") ?></h3>
		<input type="text" name="data[INFO_PROJECT_CODE]" value="<?= tpl_value($data['INFO_PROJECT_CODE']) ?>" size="64" maxlength="64" />
		<div class="Info"><?= tpl_text('A subcode or number identifying a specific project') ?></div>
	</div>

	<div class="OptionalField<? if(isset($errors['INFO_LIABLE'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text("Liable person") ?></h3>
		<input type="text" name="data[INFO_LIABLE]" value="<?= tpl_value($data['INFO_LIABLE']) ?>" size="64" maxlength="255" />
		<div class="Info"><?= tpl_text('The user primarily responsible for this cost center') ?></div>
	</div>

	<div class="OptionalField<? if(isset($errors['INFO_STREET'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text("Street address") ?></h3>
		<input type="text" name="data[INFO_STREET]" value="<?= tpl_value($data['INFO_STREET']) ?>" size="64" maxlength="64" />
	</div>

	<div class="OptionalField<? if(isset($errors['INFO_ZIP'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text("Zip code") ?></h3>
		<input type="text" name="data[INFO_ZIP]" value="<?= tpl_value($data['INFO_ZIP']) ?>" size="64" maxlength="64" />
	</div>

	<div class="OptionalField<? if(isset($errors['INFO_CITY'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text("City") ?></h3>
		<input type="text" name="data[INFO_CITY]" value="<?= tpl_value($data['INFO_CITY']) ?>" size="64" maxlength="64" />
	</div>

	<div class="OptionalField<? if(isset($errors['INFO_COUNTRY'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text("Country") ?></h3>
		<input type="text" name="data[INFO_COUNTRY]" value="<?= tpl_value($data['INFO_COUNTRY']) ?>" size="64" maxlength="64" />
	</div>

	<div class="OptionalField<? if(isset($errors['INFO_EMAIL'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text("Contact email address") ?></h3>
		<input type="text" name="data[INFO_EMAIL]" value="<?= tpl_value($data['INFO_EMAIL']) ?>" size="64" maxlength="64" />
	</div>

	<div class="OptionalField<? if(isset($errors['INFO_PHONE'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text("Contact phone number") ?></h3>
		<input type="text" name="data[INFO_PHONE]" value="<?= tpl_value($data['INFO_PHONE']) ?>" size="64" maxlength="64" />
	</div>

	<div class="OptionalField<? if(isset($errors['INFO_FAX'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text("Fax number") ?></h3>
		<input type="text" name="data[INFO_FAX]" value="<?= tpl_value($data['INFO_FAX']) ?>" size="64" maxlength="64" />
	</div>

	<span title="<?= tpl_text('Accesskey: %s','S') ?>">
		<input accesskey="s" type="submit" class="button" name="post" value="<?= tpl_text('Save') ?>" />
	</span>
	<input type="button" class="button" value="<?= tpl_text('Abort') ?>"  
		onclick="window.location='<?= tpl_uri_return() ?>';" />
	<? if (!$node->isNew()) { ?>
		<? $parent = $node->getParent(); ?>
		<input class="button" type="button" value="<?= tpl_text('Delete') ?>" 
			onclick="window.location='<?= tpl_view_call('node','delete',$node->nodeId) ?>';" />
	<? } ?>
</div>