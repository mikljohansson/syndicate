<div class="Article">
	<div class="Header">
		<h1><?= $node->toString() ?></h1>
	</div>
	<div class="Body">
		<?= $node->getMarkup() ?>
	</div>

	<h2><?= tpl_text('Search the wiki') ?></h2>
	<form action="<?= tpl_link('wiki','search',$node->wikiId()) ?>" method="get">
		<input type="hidden" name="redirect" value="1" />
		<input type="text" name="query" size="12" />
		<input type="submit" class="button" value="<?= tpl_text('Search') ?>" />
	</form>
</div>
