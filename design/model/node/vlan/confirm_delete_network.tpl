<form method="post">
	<div class="Confirm">
		<h2><?= tpl_text('Really delete this network?') ?></h2>
		<input type="submit" name="confirm" value="<?= tpl_text('Ok') ?>" />
		<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
	</div>
</form>