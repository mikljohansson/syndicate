<? $inventory = Module::getInstance('inventory'); ?>
<div class="Success">
	<h3><?= tpl_text('New leasing complete') ?></h3>
	<div class="Info">
		<? if (null != $inventory->getPrinter()) { ?>
			<?= tpl_translate("Two copies of the lease has been printed on %s. Have the client sign both, keep one and give the other to the client. You can selected a different printer from the menu and <a href=\"%s\">print the leases again</a>. When handing out a computer or laptop it should always be ghosted with a new OS install.", 
				tpl_quote($inventory->getPrinter()), tpl_view_call($lease->getHandler(),'invoke',$lease->nodeId,'printReceipt')) ?>
		<? } else { ?>
			<?= tpl_text("No valid printer selected to print lease.") ?>
		<? } ?>
	</div>
	<ul class="Actions">
		<li><a href="<?= tpl_link('inventory','newLeasing') ?>">
			<?= tpl_text('Hand out another item') ?></a></li>
	</ul>
</div>

<div class="Article">
	<h3><?= tpl_text('Client lease') ?></h3>
	<? $this->render($lease,'list_view.tpl') ?>

	<h3><?= tpl_text('Item that has been leased') ?></h3>
	<div class="Info">
		<?= tpl_translate('Give this item to %s.', $this->fetchnode($item->getCustomer(),'head_view.tpl')) ?>
	</div>
	<? $this->render($item,'list_view.tpl') ?>
</div>
