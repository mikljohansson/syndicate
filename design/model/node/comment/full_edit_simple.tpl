<? global $synd_user; ?>
<script type="text/javascript">
<!--
function updateCharsLeft(oInput){
	var iMaxLength = <?= tpl_def($maxlength,700) ?>;
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
<form action="<?= tpl_view_call('node','insert') ?>" method="post" style="margin:0;">
	<? if ($synd_user->isNull()) { ?>
	<div class="-fixed"><?= tpl_text('Name') ?></div>
	<div class="indent">
		<input type="text" name="data[CLIENT_NAME]" style="<?= tpl_def($input_style,'width:235px;') ?>;" size="16" maxlength="40" />
	</div>
	<div style="margin-top:5px;"></div>
	<? } ?>

	<div class="indent">
		<input type="hidden" name="class_id" value="comment" />
		<input type="hidden" name="data[PARENT_NODE_ID]" value="<?= $parent_node_id ?>" />

		<textarea name="data[INFO_BODY]" style="<?= tpl_def($input_style,'width:235px; height:50px') ?>;" onkeyup="updateCharsLeft(this);" cols="40" rows="4"></textarea>
		<div style="margin-top:5px;"></div>
		<input type="submit" class="button" name="post" value="<?= tpl_text('Post') ?>" />
		<input type="text" value="<?= tpl_def($maxlength,700) ?>" size="3" name="msgCL" disabled="disabled" />
	</div>
</form>