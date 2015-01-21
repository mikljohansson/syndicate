<div class="Article">
	<? include tpl_design_path('gui/errors.tpl'); ?>
	
	<div class="RequiredField">
		<h3><?= tpl_text('Title') ?></h3>
		<input type="text" name="data[INFO_HEAD]" value="<?= tpl_value($node->getTitle()) ?>" 
			size="80" maxlength="255" style="width:500px;" />
	</div>
	<div class="RequiredField">
		<h3><?= tpl_text('Short description') ?></h3>
		<?= tpl_form_textarea('data[INFO_DESC]',$node->getDescription(), array('cols' => 60)) ?>
	</div>
	<div class="RequiredField">
		<h3><?= tpl_text('Text') ?></h3>
		<? $this->render($node->getBody(),'full_edit.tpl',array(
			'id'    => 'data[INFO_BODY]', 
			'style' => array('width'=>'500px'))) ?>
	</div>
	<div class="OptionalField">
		<h3><?= tpl_text("Course identifier (used in url's)") ?></h3>
		<input type="text" name="data[INFO_COURSE_ID]" value="<?= tpl_value($data['INFO_COURSE_ID']) ?>" 
			size="80" maxlength="64" style="width:500px;" />
	</div>
	<div class="OptionalField">
		<?= tpl_form_checkbox('data[FLAG_DIAGNOSTIC]', $node->hasDiagnosticTest()) ?>
			<label for="data[FLAG_DIAGNOSTIC]"><?= tpl_text("Enable initial 'Diagnostic-Test'") ?></label>
	</div>

	<span title="<?= tpl_text('Accesskey: %s','S') ?>">
		<input accesskey="s" type="submit" name="post" value="<?= tpl_text('Save') ?>" />
	</span>
	<input type="button" class="button" value="<?= tpl_text('Abort') ?>"  
		onclick="window.location='<?= tpl_uri_return() ?>';" />
	<? if (!$node->isNew()) { ?>
	<? $parent = $node->getParent(); ?>
	<input class="button" type="button" value="<?= tpl_text('Delete') ?>" 
		onclick="window.location='<?= tpl_view_call($node->getHandler(),'delete',$node->nodeId) ?>';" />
	<? } ?>
</div>