<input type="hidden" name="stack[]" value="<?= tpl_view($node->getHandler(),'edit',$node->nodeId,'step2') ?>" />
<div class="Article">
	<? include tpl_design_path('gui/errors.tpl') ?>
	<p>
		<?= tpl_text('Username') ?><? if (isset($errors['USERNAME'])) print '<span style="color:red;">*</span>'; ?><br />
		<input type="text" name="data[USERNAME]" value="<?= tpl_value($data['USERNAME']) ?>" size="32" maxlength="32" />
	</p>
	<p>
		<?= tpl_text('Password') ?><? if (isset($errors['PASSWORD'])) print '<span style="color:red;">*</span>'; ?><br />
		<input type="password" name="data[PASSWORD]" value="<?= tpl_value($data['PASSWORD']) ?>" size="32" maxlength="32" />
	</p>
	<p>
		<?= tpl_text('Password') ?><? if (isset($errors['PASSWORD2'])) print '<span style="color:red;">*</span>'; ?><br />
		<input type="password" name="data[PASSWORD2]" value="<?= tpl_value($data['PASSWORD2']) ?>" size="32" maxlength="32" />
	</p>
	<p><input class="button" type="submit" name="post" value="<?= tpl_text('Signup') ?>" /></p>
</div>