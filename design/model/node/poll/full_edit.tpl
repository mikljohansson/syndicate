<div class="Article">
	<div class="RequiredField">
		<h3><?= tpl_text('Title') ?></h3>
		<input type="text" name="data[INFO_HEAD]" value="<?= tpl_value($data['INFO_HEAD']) ?>" style="width:600px;" maxlength="255" />
	</div>

	<div class="RequiredField">
		<h3><?= tpl_text('Short description') ?></h3>
		<?= tpl_form_textarea('data[INFO_DESC]',$data['INFO_DESC'],
			array('rows' => 2,'style'=>'width:600px')) ?>
	</div>

	<div class="RequiredField">
		<h3><?= tpl_text('Introductory text') ?></h3>
		<? $this->render($node->getBody(),'full_edit.tpl',array(
			'id'    => 'data[INFO_BODY]', 
			'style' => array('width'=>'600px'))) ?>
	</div>

	<div class="OptionalField">
		<h3><?= tpl_text('Confirmation text') ?></h3>
		<div class="Info"><?= tpl_text('Displayed after user has submitted his/her answers.') ?></div>
		<? $this->render($data['confirm']['INFO_BODY'],'full_edit.tpl',array(
			'id'    => 'data[confirm][INFO_BODY]', 
			'style' => array('width'=>'600px'))) ?>
	</div>

	<div class="OptionalField">
		<h3><?= tpl_text('Redirect after reply') ?></h3>
		<div class="Info"><?= tpl_text("If specified; clients are redirected here after replying. For example <em>'http://www.example.com'</em>") ?></div>
		<input type="text" name="data[INFO_REDIRECT]" value="<?= tpl_value($data['INFO_REDIRECT']) ?>" style="width:600px;" maxlength="255" />
	</div>

	<div class="OptionalField">
		<h3><?= tpl_text('Error text') ?></h3>
		<div class="Info"><?= tpl_text('Displayed after the poll is closed or when the user does not have permission to reply.') ?></div>
		<? $this->render($data['error']['INFO_BODY'],'full_edit.tpl',array(
			'id'    => 'data[error][INFO_BODY]', 
			'style' => array('width'=>'600px'))) ?>
	</div>

	<input type="submit" class="button" name="post" value="<?= tpl_text('Save') ?>" />
	<input type="button" class="button" value="<?= tpl_text('Abort') ?>"  onclick="window.location='<?= tpl_uri_return() ?>';" />
	<? if (!$node->isNew()) { ?>
	<input class="button" type="button" value="<?= tpl_text('Delete') ?>" 
		onclick="window.location='<?= tpl_view_call($node->getHandler(),'delete',$node->nodeId,
			tpl_view($node->getHandler())) ?>';" />
	<? } ?>
</div>