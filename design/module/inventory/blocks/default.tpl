<? $menu->display(); ?>
<? if (count($module->getPrinters())) { ?>
<form action="<?= tpl_view_call('system','setEnvironment') ?>" method="post">
	<h4><?= tpl_text('Printer') ?></h4>
	<select name="senv[inventory][printer]" onchange="this.form.submit()">
		<?= tpl_form_options($module->getPrinters(), $module->getPrinter()) ?>
	</select>
</form>
<? } ?>