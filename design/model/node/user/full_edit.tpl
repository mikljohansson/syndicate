<script type="text/javascript">
<!--
function updateCharsLeft(oInput, iMaxLength){
	var iLeft = 0;
	if (oInput.value.length > iMaxLength) {
		oInput.value = oInput.value.substring(0,iMaxLength);
	} else {
		iLeft = iMaxLength - oInput.value.length
	}
	oInput.form.msgCL.value = iLeft;
}
//-->
</script>
<table class="Article">
<tr>
	<td class="left" style="width:200px;">
		<? include tpl_design_path('gui/errors.tpl'); ?>
		
		<?= tpl_text('Username') ?><? if (isset($errors['USERNAME'])) print '<span style="color:red;">*</span>'; ?>
		<div class="indent">
			<input type="text" name="data[USERNAME]" value="<?= tpl_value($data['USERNAME']) ?>" style="width:200px;" maxlength="32" />
		</div>

		<?= tpl_text('Public name') ?><? if (isset($errors['INFO_HEAD'])) print '<span style="color:red;">*</span>'; ?>
		<div class="indent">
			<input type="text" name="data[INFO_HEAD]" value="<?= tpl_value($data['INFO_HEAD']) ?>" style="width:200px;" maxlength="32" />
		</div>

		<?= tpl_text('Email address') ?><? if (isset($errors['INFO_EMAIL'])) print '<span style="color:red;">*</span>'; ?>
		<div class="indent">
			<input type="text" name="data[INFO_EMAIL]" style="width:200px;" maxlength="512" />
		</div>

		<?= tpl_text('Password') ?><? if (isset($errors['PASSWORD2'])) print '<span style="color:red;">*</span>'; ?>
		<div class="indent">
			<input type="password" name="data[PASSWORD]" style="width:200px;" maxlength="32" />
		</div>

		<?= tpl_text('Password') ?><? if (isset($errors['PASSWORD2'])) print '<span style="color:red;">*</span>'; ?>
		<div class="indent">
			<input type="password" name="data[PASSWORD2]" style="width:200px;" maxlength="32" />
		</div>


		<br />
		<div class="indent nowrap">
			<input class="button" type="submit" name="post" value="<?= tpl_text('Save') ?>" />
			<input class="button" type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
		</div>

		<b><?= tpl_text('Note:') ?></b>
		<div class="indent">
			<?= tpl_text('<b>Email</b> is used for password retrival purposes and will not be shown publically or sold to third parties.') ?>
			<br /><br />
			<?= tpl_text('<b>Description</b> will be displayed alongside <b>Photo</b> on public pages') ?>
		</div>
	</td>
	<td>&nbsp;</td>
	<td class="left" style="width:350px;">
		<?= tpl_text('Description') ?><? if (isset($errors['INFO_DESC'])) print '<span style="color:red;">*</span>'; ?>
		<div class="indent">
			<textarea name="data[INFO_DESC]" style="width:348px;height:40px;" onkeyup="updateCharsLeft(this, <?= $node->_max_length_desc ?>);"><?= $data['INFO_DESC'] ?></textarea>
			<input style="display:none;" type="text" value="<?= $node->_max_length_desc - strlen($data['INFO_DESC']) ?>" size="3" name="msgCL" disabled="disabled" />
		</div>

		<?= tpl_text('Homepage text') ?><? if (isset($errors['INFO_BODY'])) print '<span style="color:red;">*</span>'; ?>
		<div class="indent">
			<? if (is_object($data['INFO_BODY'])) { ?>
				<? $this->render($node->data['INFO_BODY'],'full_edit.tpl',
					array('id'=>'data[INFO_BODY]', 'style'=>array('width'=>'350px','height'=>'200px'))) ?>
			<? } else { ?>
				<textarea name="data[INFO_DESC]" style="width:348px;height:200px;"><?= $data['INFO_BODY'] ?></textarea>
			<? } ?>
		</div>
	</td>
</tr>
</table>
