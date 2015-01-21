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
	<div class="RequiredField<? if(isset($errors['INFO_URI'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Link to agreement text') ?></h3>
		<input type="text" name="data[INFO_URI]" value="<?= tpl_value($data['INFO_URI']) ?>" size="64" maxlength="1024" />
	</div>
	<div class="RequiredField<? if(isset($errors['PARENT_NODE_ID'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Parent folder') ?></h3>
		<select name="data[PARENT_NODE_ID]">
			<? $this->display(tpl_design_path('module/inventory/folder_options.tpl'),
				array('selected' => SyndNodeLib::getInstance($data['PARENT_NODE_ID']))); ?>
		</select>
	</div>
	<p>
		<span title="<?= tpl_text('Accesskey: %s','S') ?>">
			<input accesskey="s" type="submit" class="button" name="post" value="<?= tpl_text('Save') ?>" />
		</span>
		<span title="<?= tpl_text('Accesskey: %s','A') ?>">
			<input accesskey="a" type="button" class="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
		</span>
		<? if (!$node->isNew()) { ?>
		<input class="button" type="button" value="<?= tpl_text('Delete') ?>" onclick="window.location='<?= 
			tpl_view_call('inventory','delete',$node->nodeId,array('redirect' => tpl_view('inventory','view',$node->getParent()->nodeId))) ?>';" />
		<? } ?>
	</p>
</div>