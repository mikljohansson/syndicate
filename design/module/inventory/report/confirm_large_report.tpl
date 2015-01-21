<div class="Notice">
	<h2><?= tpl_text('Please note') ?></h2>
	<div class="indent">
		<?= tpl_text('The report you are generating will contain upwards to %d items, which might take a long time to generate. Do you want to continue?', $collection->getCount()) ?>
	</div>
	<form method="post">
		<? foreach (tpl_form_implode($_POST) as $key => $value) { ?>
		<input type="hidden" name="<?= $key ?>" value="<?= tpl_attribute($value) ?>" />
		<? } ?>
		
		<div style="margin-top:1em;">
			<input type="submit" name="confirm" value="<?= tpl_text('Ok') ?>" />
			<input type="button" value="<?= tpl_text('Abort') ?>" onclick="history.go(-1);" />
		</div>
	</form>
</div>