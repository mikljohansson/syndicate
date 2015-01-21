<? tpl_load_script(tpl_design_uri('js/form.js')) ?>
<div class="Article" style="width:600px;">
	<? include tpl_design_path('gui/errors.tpl'); ?>
	
	<div class="RequiredField<? if(isset($errors['INFO_MAKE'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Maker') ?></h3>
		<input type="text" name="data[INFO_MAKE]" match="\w+" message="<?= tpl_text('Please provide a maker') ?>" 
			value="<?= tpl_value($data['INFO_MAKE']) ?>" size="64" maxlength="512" />
	</div>
	<div class="RequiredField<? if(isset($errors['INFO_PRODUCT'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Product name') ?></h3>
		<input type="text" name="data[INFO_PRODUCT]" match="\w+" message="<?= tpl_text('Please provide a product name') ?>" 
			value="<?= tpl_value($data['INFO_PRODUCT']) ?>" size="64" maxlength="512" />
	</div>

	<div class="RequiredField<? if(isset($errors['PARENT_NODE_ID'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Parent folder') ?></h3>
		<select name="data[PARENT_NODE_ID]">
			<? $this->display(tpl_design_path('module/inventory/folder_options.tpl'),
				array('selected' => SyndNodeLib::getInstance($data['PARENT_NODE_ID']))); ?>
		</select>
	</div>
	
	<div class="RequiredField<? if(isset($errors['INFO_LICENSES'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Number of licenses') ?></h3>
		<input type="text" name="data[INFO_LICENSES]" value="<?= tpl_value($data['INFO_LICENSES']) ?>" size="64" maxlength="64" />
	</div>
	<div class="OptionalField<? if(isset($errors['FLAG_SITE_LICENSE'])) print ' InvalidField'; ?>">
		<?= tpl_form_checkbox('data[FLAG_SITE_LICENSE]',$node->isSiteLicense()) ?>
			<label for="data[FLAG_SITE_LICENSE]"><?= tpl_text('This is a site (unlimited) license') ?></label>
	</div>

	<div class="OptionalField<? if(isset($errors['INFO_DESC'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Description') ?></h3>
		<?= tpl_form_textarea('data[INFO_DESC]',$data['INFO_DESC'],array('cols'=>'48')) ?>
	</div>
	
	<span title="<?= tpl_text('Accesskey: %s','S') ?>">
		<input accesskey="s" type="submit" name="post" value="<?= tpl_text('Save') ?>" />
	</span>
	<span title="<?= tpl_text('Accesskey: %s','A') ?>">
		<input accesskey="a" type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
	</span>
	<? if (!$node->isNew()) { ?>
		<? $parent = $node->getParent(); ?>
		<input class="button" type="button" value="<?= tpl_text('Delete') ?>" onclick="window.location='<?= 
			tpl_view_call($node->getHandler(),'delete',$node->nodeId, tpl_view($parent->getHandler(),'view',$parent->nodeId)) ?>';" />
	<? } ?>
</div>