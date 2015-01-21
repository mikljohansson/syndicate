<? global $synd_user; $inventory = Module::getInstance('inventory'); ?>
<h1><?= $this->text('Inventory') ?></h1>
<dl class="Actions">
	<dt><a href="<?= $this->href('inventory','flow','repair','1') ?>"><?= $this->text('Recieve item for repair') ?></a></dt>
	<dd><?= $this->text('Repair that does not require the client to have his/her equipment replaced. For example reinstalling the operating system.') ?></dd>
	<dt><a href="<?= $this->href('inventory','flow','replace','1') ?>"><?= $this->text('Replacement of faulty item') ?></a></dt>
	<dd><?= $this->text('Extended repair that requires equipment to be shipped off to be repaired. The client will in most cases receive a replacement item.') ?></dd>
	<dt><a href="<?= $this->href('inventory','report','batch_resolve_issues') ?>"><?= $this->text('Resolve active repair issues') ?></a></dt>
	<dd><?= $this->text('Use when handing a repaired item back to the client or accepting a batch of items back from external repairs.') ?></dd>
</dl>
<ul class="Actions">
	<li class="Caption"><?= $this->text('Lease maintenance') ?></li>
	<li><a href="<?= $this->href('inventory','newLeasing') ?>"><?= $this->text('Hand out item on a lease') ?></a></li>
	<li><a href="<?= $this->href('inventory','flow','terminate','1') ?>"><?= $this->text('Terminate a lease') ?></a></li>
	<li><a href="<?= $this->href('inventory','payInvoices') ?>"><?= $this->text('Mark invoices as paid') ?></a></li>
	<li><a href="<?= $this->href('inventory','report','invoices') ?>"><?= $this->text('Show pending invoices') ?></a></li>
</ul>
<ul class="Actions">
	<li class="Caption"><?= $this->text('Reports and listings') ?></li>
	<li><a href="<?= $this->href('inventory','report','unresolved_issues') ?>"><?= $this->text('Open issues') ?></a></li>
	<li><a href="<?= $this->href('inventory','report','non_warranty_issues') ?>"><?= $this->text('Non warranty issues') ?></a></li>
	<li><a href="<?= $this->href('inventory','report','excessive_client_repairs') ?>"><?= $this->text('Clients with many repairs') ?></a></li>
	<li><a href="<?= $this->href('inventory','report','duplicates') ?>"><?= $this->text('Serial number duplicates') ?></a></li>
</ul>
<ul class="Actions">
	<li class="Caption"><?= $this->text('Batch import') ?></li>
	<li><a href="<?= $this->href('inventory','importItems') ?>"><?= $this->text('Batch import items from CSV') ?></a></li>
	<li><a href="<?= $this->href('inventory','importLeases') ?>"><?= $this->text('Batch import leases from CSV') ?></a></li>
</ul>