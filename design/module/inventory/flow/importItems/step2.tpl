<div class="Success">
	<h2><?= tpl_text('Imported %d items', count($items)) ?></h2>
</div>

<h3><?= tpl_text('Imported items') ?></h3>
<?= tpl_gui_table('item',$items,'view.tpl',array('hideCheckbox'=>true)) ?>