<form method="post">
	<div class="Success">
		<h1><?= tpl_text('Moved the selected items to %s.', $this->fetchnode($folder,'head_view.tpl')) ?></h1>
		<? if ($printer) { ?>
		<p><?= tpl_text("A receipt has been printed on <em>'%s'</em>. You can selected a different printer from the menu and print again.", $printer) ?></p>
		<p>
			<input type="hidden" name="collections[]" value="<?= $collection->id() ?>" />
			<input type="hidden" name="folder" value="<?= $folder->nodeId ?>" />
			<input type="hidden" name="receipt" value="<?= $receipt->nodeId ?>" />
			<input type="submit" name="confirm" value="<?= tpl_text('Print receipt again') ?>" />
		</p>
		<? } ?>
	</div>
</form>
