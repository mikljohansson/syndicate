<div class="Article">
	<div class="Success">
		<h1><?= tpl_text('Created %d new invoices', (int)$request['count']) ?></h1>
		<h3><?= $node->getTitle() ?></h3>
		<div class="Info">
			<?= tpl_text('Created on %s', ucwords(tpl_strftime('%d %b %Y', $node->data['TS_CREATE']))) ?>, 
			<?= tpl_text('Due on %s', ucwords(tpl_strftime('%d %b %Y', $node->data['TS_RESOLVE_BY']))) ?>,
			<?= tpl_text('Amount %d', $node->getAmount()) ?>
		</div>
	</div>
	<?= tpl_gui_table('lease',$collection->getFilteredContents(array('synd_node_lease')),'view.tpl') ?>
</div>
