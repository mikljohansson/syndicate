<div class="Article">
	<div class="Header">
		<h1><?= tpl_text('Confirm remove') ?></h1>
		<div class="Info">
			<?= tpl_text('Do you really want to remove the following questions from this test?') ?>
		</div>
	</div>
</div>

<form method="post">
	<input type="hidden" name="collections[]" value="<?= $collection->id() ?>" />
	<? $this->iterate($collection->getFilteredContents(array('synd_node_relation')), 'list_view.tpl') ?>

	<input type="submit" name="confirm" value="<?= tpl_text('Remove') ?>" />
	<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
</form>
