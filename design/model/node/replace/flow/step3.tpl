<? assert('$node->isValid()') ?>
<? if ($node->isValid()) { ?>
	<div class="Success">
		<h3><?= tpl_text('Replacement issue complete') ?></h3>
		<div class="Info">
			<? if (!empty($request['receipt'])) { ?>
				<? if (null != $node->getPrinter()) { ?>
					<?= tpl_translate("A receipt has been printed on %s, give this to the client. You can selected a different printer from the menu and <a href=\"%s\">print receipt again</a>.", 
						tpl_quote($node->getPrinter()), tpl_view_call($node->getHandler(),'invoke',$node->nodeId,'printReceipt')) ?>
				<? } else { ?>
					<?= tpl_text("No valid printer selected to print receipt.") ?>
				<? } ?>
			<? } ?>
			<?= tpl_translate('An <a href="%s">issue</a> regarding this replacement has been created.', 
				tpl_view($node->getHandler(),$node->objectId())) ?>
		</div>
	</div>

	<div class="Article">
		<h3><?= tpl_text('Item that has been replaced') ?></h3>
		<? 
		$inventory = Module::getInstance('inventory'); 
		if (null != $inventory->getRepairFolder()) { ?>
		<div class="indent">
			<?= tpl_translate('Place this item on the %s shelf.', 
				$this->fetchnode($inventory->getRepairFolder(),'head_view.tpl')) ?>
		</div>
		<? } ?>
		<div class="indent">
			<? $this->iterate($node->getItems(),'list_view.tpl') ?>
		</div>

		<h3><?= tpl_text('Replacement item') ?></h3>
		<div class="indent">
			<?= tpl_text('Give this item to %s.', 
				$this->fetchnode($node->getCustomer(),'head_view.tpl')) ?>
		</div>
		<div class="indent">
			<? $this->iterate($node->getReplacements(),'list_view.tpl') ?>
		</div>
	</div>
<? } else { ?>
	<div class="Warning">
		<h2><?= tpl_text('An error occurred') ?></h2>
		<?= tpl_text('Please contact your systems administrator.') ?>
	</div>
	<div class="Article">
		<h3><?= tpl_text('Item to replace') ?></h3>
		<div class="indent">
			<? $this->iterate($node->getItems(),'list_view.tpl') ?>
		</div>

		<h3><?= tpl_text('Replacement item') ?></h3>
		<div class="indent">
			<? $this->iterate($node->getReplacements(),'list_view.tpl') ?>
		</div>
	</div>
<? } ?>
