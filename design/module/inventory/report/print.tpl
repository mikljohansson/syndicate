<?
$inventory = Module::getInstance('inventory');
$printer = $inventory->getPrinter();
$section = array('synd_node_invoice','synd_node_repair','synd_node_lease');

if (null == $collection || 0 == $collection->getFilteredCount($section)) { ?>
	<div class="Notice">
		<h2><?= tpl_text('No invoices, repairs or leases selected to print. Please make a selection and try again.') ?></h2>
		<p><a href="<?= tpl_uri_return() ?>"><?= tpl_text('Back to selection') ?></a></p>
	</div>
<? } else if (isset($request['confirm'])) {
	set_time_limit(3600);

	$failed = array();
	foreach (array_keys($list = SyndLib::filter($collection->getFilteredContents($section),'isPermitted','read')) as $key) {
		if (!$list[$key]->toPrinter($printer))
			$failed[] = $list[$key];
	}

	if (empty($failed)) { ?>
		<div class="Success">
			<?= tpl_text('Printed %d receipts to printer <em>%s</em>', count($list), $printer) ?>
		</div>
		<a href="<?= tpl_uri_return() ?>"><?= tpl_text('Back to selection') ?></a>		
	<? } else { ?>
		<form method="post">
			<input type="hidden" name="collections[]" value="<?= $collection->id() ?>" />
			<div class="Warning">
				<p><?= tpl_text('Failed printing of the following %d receipts to printer <em>%s</em> (printed %d receipt successfully)', 
					count($failed), $printer, count($list)-count($failed)) ?></p>
				<input type="submit" name="confirm" value="<?= tpl_text('Try again') ?>" />
				<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
			</div>
		</form>
		<? $this->iterate($failed,'item.tpl') ?>
	<? } ?>
<? } else { ?>
	<form method="post">
		<input type="hidden" name="collections[]" value="<?= $collection->id() ?>" />
		<div class="Notice">
			<p><?= tpl_text('Do you really want to print %d receipts to the printer <em>%s</em>, this might take a while to complete', 
				$collection->getFilteredCount($section), $printer) ?></p>
			<input type="submit" name="confirm" value="<?= tpl_text('Ok') ?>" />
			<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
		</div>
	</form>
<? } ?>