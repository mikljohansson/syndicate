	<div class="RequiredField">
		<h3><?= tpl_text('Title') ?></h3>
		<input type="text" name="data[INFO_HEAD]" value="<?= tpl_value($data['INFO_HEAD']) ?>" style="width:600px;" maxlength="255" />
	</div>

	<div class="RequiredField">
		<h3><?= tpl_text('Short description') ?></h3>
		<input type="text" name="data[INFO_DESC]" value="<?= tpl_value($data['INFO_DESC']) ?>" style="width:600px;" maxlength="512" />
	</div>

	<div class="RequiredField">
		<h3><?= tpl_text('Text') ?></h3>
		<? $this->render($data['INFO_BODY'],'full_edit.tpl',array(
			'id'    => 'data[INFO_BODY]', 
			'style' => array('width'=>'600px'))) ?>
	</div>

	<div class="OptionalField">
		<?= tpl_form_checkbox('data[FLAG_PROGRESS]', $data['FLAG_PROGRESS']) ?>
			<label for="data[FLAG_PROGRESS]"><?= tpl_text("Enable 'Progress-Checks' (student examination)") ?></label><br />
		<?= tpl_form_checkbox('data[FLAG_DIAGNOSTIC]', $data['FLAG_DIAGNOSTIC']) ?>
			<label for="data[FLAG_DIAGNOSTIC]"><?= tpl_text("Enable 'Diagnostic-Test' (initial student examination)") ?></label>
	</div>

	<input type="submit" class="button" name="post" value="<?= tpl_text('Save') ?>" />
	<input type="button" class="button" value="<?= tpl_text('Abort') ?>" 
		onclick="window.location='<?= tpl_view($node->getHandler(),'view',$node->nodeId) ?>';" />
	<? if (!$node->isNew()) { ?>
	<? $parent = $node->getParent(); ?> 
	<input class="button" type="button" value="<?= tpl_text('Delete') ?>" 
		onclick="window.location='<?= tpl_view_call($node->getHandler(),'delete',$node->nodeId,
			array('redirect' => tpl_view($parent->getHandler(),'view',$parent->nodeId))) ?>';" />
	<? } ?>
