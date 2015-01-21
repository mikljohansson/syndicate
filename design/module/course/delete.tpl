<h3><?= tpl_text('Really delete?') ?></h3>
<? $this->iterate($contents,'item.tpl') ?>

<form method="post">
	<? if (null !== $collection) { ?>
	<input type="hidden" name="collections[]" value="<?= $collection->id() ?>" />
	<? } ?>
	<? foreach (array_keys($contents) as $key) { ?>
	<input type="hidden" name="selection[]" value="<?= $contents[$key]->id() ?>" />
	<? } ?>
	<p>
		<input type="submit" name="post" value="<?= tpl_text('Delete') ?>" />
		<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>'" />
	</p>
</form>
