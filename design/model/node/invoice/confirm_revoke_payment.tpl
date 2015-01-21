<form method="post">
	<div class="Confirm">
		<h2><?= tpl_text('Revoke invoice payment') ?></h2>
		<p><?= tpl_text('Do you really want to mark this invoice as not paid?') ?></p>
		<input type="submit" name="confirm" value="<?= tpl_text('Ok') ?>" />
		<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
	</div>
</form>