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
<form action="<?= tpl_view_call('node','insert') ?>" method="post">
	<? if ($synd_user->isNull()) { ?>
	<?= tpl_text('Name') ?>
	<div class="indent">
		<input type="text" name="data[CLIENT_NAME]" style="width:260px;" maxlength="40" />
	</div>
	<? } ?>

	<div class="indent">
		<input type="hidden" name="class_id" value="comment" />
		<input type="hidden" name="data[PARENT_NODE_ID]" value="<?= $parent_node_id ?>" />

		<textarea name="data[INFO_BODY]" style="width:260px;" cols="29" rows="3"></textarea><br />
		<input type="submit" class="button" name="post" value="<?= tpl_text('Post') ?>" />
		<input type="text" value="<?= tpl_def($maxlength,700) ?>" size="3" name="msgCL" disabled="disabled" />
	</div>
</form>