<div class="Success">
	<h2><?= tpl_text('Imported %d leases', count($nodes)) ?></h2>
</div>

<h3><?= tpl_text('Imported leases') ?></h3>
<?= tpl_gui_table('lease',$nodes,'view.tpl',array('hideCheckbox'=>true)) ?>