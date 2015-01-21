<form method="post">
	<input type="hidden" name="confirm" value="1" />
	<b><?= tpl_text("Do you really want to delete this file?") ?></b>
	<? $this->render($file,'full_view.tpl') ?><br />
	<input type="submit" value="<?= tpl_text('Ok') ?>" />
	<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
</form>