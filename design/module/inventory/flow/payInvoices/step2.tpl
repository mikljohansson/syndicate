<div class="Success">
	<h2><?= tpl_text('Marked %d invoices as paid', count($invoices)) ?></h2>
</div>

<h3><?= tpl_text('Invoices') ?></h3>
<? $this->display('model/node/invoice/table.tpl',array('list'=>$invoices,'hideCheckbox'=>true)) ?>