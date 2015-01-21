<? global $synd_user; ?>
<h1><?= tpl_text('Inventory') ?></h1>

<dl class="Actions">
	<dt><a href="<?= tpl_link('inventory','flow','repair','1') ?>"><?= tpl_text('Recieve item for repair') ?></a></dt>
	<dt><a href="<?= tpl_link('inventory','flow','replace','1') ?>"><?= tpl_text('Replacement of faulty item') ?></a></dt>
</dl>

<form action="<?= tpl_view('inventory','search') ?>" method="get">
	<input type="hidden" name="redirect" value="1" />
	<input type="text" name="query" size="8" />
	<input type="submit" name="post" value="<?= tpl_text('Search') ?>" />
</form>