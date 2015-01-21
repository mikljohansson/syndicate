<form action="<?= tpl_link('search') ?>" method="get">
	<span title="<?= tpl_text('Accesskey: %s', 'F') ?>">
		<input accesskey="f" type="text" name="q" size="16" />
		<input type="submit" value="<?= tpl_text('Search') ?>" />
	</span>
</form>