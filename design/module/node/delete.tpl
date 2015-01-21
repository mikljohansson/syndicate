<div class="Article">
	<form method="post">
		<h2><?= tpl_text('Really delete?') ?></h2>
		<? if (null != tpl_gui_path(get_class($node),'item.tpl',false)) { ?>
			<? $this->render($node,'item.tpl') ?>
		<? } else { ?>
			<? $this->render($node,'list_view.tpl') ?>
		<? } ?>

		<p>
			<span title="<?= tpl_text('Accesskey: %s','S') ?>">
				<input accesskey="s" type="submit" name="confirm" value="<?= tpl_text('Ok') ?>" />
			</span>
			<span title="<?= tpl_text('Accesskey: %s','A') ?>">
				<input accesskey="a" type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
			</span>
		</p>
	</form>
</div>
