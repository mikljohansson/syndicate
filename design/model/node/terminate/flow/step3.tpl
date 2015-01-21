<div class="Success">
	<h3><?= tpl_text('Lease has been terminated') ?></h3>
	<div class="Info">
		<? if (!empty($request['receipt'])) { ?>
			<? if (null != $node->getPrinter()) { ?>
				<?= tpl_translate("A receipt has been printed on %s, give this to the client. You can selected a different printer from the menu and <a href=\"%s\">print receipt again</a>.", 
					tpl_quote($node->getPrinter()), tpl_view_call($node->getHandler(),'invoke',$node->nodeId,'printReceipt')) ?>
				<? if ($node->isRepair()) { ?>
				<?= tpl_text('Also; a repair slip has been printed, attach this to the item.') ?>
				<? } ?>
			<? } else { ?>
				<?= tpl_text("No valid printer selected to print receipt.") ?>
			<? } ?>
		<? } ?>
		<?= tpl_translate('An <a href="%s">issue</a> regarding this lease termination has been created.', 
			tpl_view($node->getHandler(),$node->objectId())) ?>
	</div>
</div>

<? if (count($items = $node->getItems())) { ?>
<div class="Article">
	<h3><?= tpl_text('Items returned by client') ?></h3>
	<? 
	$inventory = Module::getInstance('inventory'); 
	if (!empty($items))
		$folder = $items[key($items)]->getFolder();
	
	if (!empty($folder)) { ?>
	<div class="Info">
		<?= tpl_translate('Place these items on the %s shelf.', $this->fetchnode($folder,'head_view.tpl')) ?>
	</div>
	<? } ?>
	<? $this->iterate($node->getItems(),'list_view.tpl') ?>
</div>
<? } ?>
