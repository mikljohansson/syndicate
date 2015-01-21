<div class="Article" style="width:200px;">
	<?= tpl_text('Password') ?><? if (isset($errors['PASSWORD'])) print '<span style="color:red;">*</span>'; ?>
	<div class="indent">
		<input type="password" name="data[PASSWORD]" style="width:200px;" maxlength="32" />
	</div>

	<?= tpl_text('Password (verification)') ?><? if (isset($errors['PASSWORD2'])) print '<span style="color:red;">*</span>'; ?>
	<div class="indent">
		<input type="password" name="data[PASSWORD2]" style="width:200px;" maxlength="32" />
	</div>

	<br />
	<div class="indent nowrap">
		<input class="button" type="submit" name="post" value="<?= tpl_text('Save') ?>" />
		<input class="button" type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
	</div>

	<?
	if (null != $errors) {
		print '<br /><div class="indent">';
		$seen = array();
		foreach ($errors as $field => $error) {
			if (isset($error['msg']) && !in_array($error['msg'], $seen)) {
				$seen[] = $error['msg'];
				print "<span style=\"color:red;\">*</span> {$error['msg']}<br />";
			}
		}
		print '</div>';
	}
	?>
</div>