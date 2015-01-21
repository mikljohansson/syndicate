<form method="post">
	<input type="hidden" name="prototype" value="<?= $node->nodeId ?>" />
	<input type="hidden" name="collections[]" value="<?= $collection->id() ?>" />
	
	<div class="Article">
		<div class="Header">
			<h1><?= tpl_text('Confirm new invoice') ?></h1>
			<div class="Info"><?= tpl_text('%d leases selected to receive invoice', 
				$collection->getFilteredCount(array('synd_node_lease'))) ?></div>
		</div>
		<? include tpl_design_path('gui/errors.tpl'); ?>
		<div class="Notice">
			<h2><?= tpl_text('Invoice template') ?></h2>
			<h3><?= $node->getTitle() ?></h3>
			<div class="Info">
				<?= tpl_text('Created on %s', ucwords(tpl_strftime('%d %b %Y', $node->data['TS_CREATE']))) ?>, 
				<?= tpl_text('Due on %s', ucwords(tpl_strftime('%d %b %Y', $node->data['TS_RESOLVE_BY']))) ?>,
				<?= tpl_text('Amount %d', $node->getAmount()) ?>
			</div>
		</div>
		<div style="margin-top:1em;">
			<input type="submit" name="post" value="<?= tpl_text('Confirm') ?> >>>" />
		</div>
	</div>
</form>