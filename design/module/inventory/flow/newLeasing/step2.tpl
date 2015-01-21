<form method="post">
	<input type="hidden" name="lease_node_id" value="<?= $lease->nodeId ?>" />
	<input type="hidden" name="item_node_id" value="<?= $item->nodeId ?>" />
	
	<div class="Article">
		<div class="Header">
			<h2><?= tpl_text('Hand out item on a lease') ?></h2>
			<div class="Info">
				<?= tpl_text('Append an item to an existing lease.') ?>
			</div>
		</div>
		<div class="Notice">
			<h3><?= tpl_text('Client lease') ?></h3>
			<? $this->render($lease,'list_view.tpl') ?>
		</div>
		<div class="Notice">
			<h3><?= tpl_text('Item to lease') ?></h3>
			<? $this->render($item,'list_view.tpl') ?>
		</div>
	</div>

	<input type="submit" name="confirm" value="<?= tpl_text('Confirm') ?>" />
	<input type="button" value="<?= tpl_text('Abort') ?>" />
</form>