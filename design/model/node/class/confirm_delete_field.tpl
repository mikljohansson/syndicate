<form method="post">
	<input type="hidden" name="confirm" value="1" />
	<?= tpl_text("Do you really want to delete the field '%s'?", $field) ?>
	<br /><br />
	<input type="submit" value=" <?= tpl_text('Ok') ?> " />
	<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
</form>