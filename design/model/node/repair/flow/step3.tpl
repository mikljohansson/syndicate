<div class="Success">
	<h2><?= tpl_text('Repair issue complete') ?></h2>
	<div class="Info">
		<? if (!empty($request['receipt'])) { ?>
			<? if (null != $node->getPrinter()) { ?>
				<?= tpl_translate("A receipt has been printed on %s, give this to the client. You can selected a different printer from the menu and <a href=\"%s\">print receipt again</a>.", 
					tpl_quote($node->getPrinter()), 
					tpl_view_call($node->getHandler(),'invoke',$node->nodeId,'printReceipt')) ?>
			<? } else { ?>
				<?= tpl_text("No valid printer selected to print receipt.") ?>
			<? } ?>
		<? } ?>
		<?= tpl_translate('An <a href="%s">issue</a> regarding this repair has been created.', 
			tpl_view($node->getHandler(),$node->objectId())) ?>
	</div>
</div>

<div class="Article">
	<h3><?= tpl_text('Item recieved for repair') ?></h3>
	<? 
	$inventory = Module::getInstance('inventory');
	if (null != ($repair = $inventory->getRepairFolder())) { 
		foreach (array_keys($items = $node->getItems()) as $key) {
			$folder = $items[$key]->getFolder();
			if ($folder->nodeId == $repair->nodeId)
				$showRepair = true;
		}
	}
	if (!empty($showRepair)) {
	?>
	<div class="indent">
		<?= tpl_translate('Place this item on the %s shelf.', 
			$this->fetchnode($inventory->getRepairFolder(),'head_view.tpl')) ?>
	</div>
	<? } ?>
	<div class="indent">
		<? $this->iterate($node->getItems(),'list_view.tpl') ?>
	</div>
</div>
