<h1><?= tpl_text('Import translations') ?></h1>
<p class="Help"><?= tpl_text('Import translations from an external Synd installation via RPC') ?></p>

<? if (!empty($errors)) { ?>
<p class="Warning">
	<?= implode('<br />', $errors) ?>
</p>
<? } ?>
<? if (!empty($status)) { ?>
<p class="Result">
	<?= implode('<br />', $status) ?>
</p>
<? } ?>

<form method="post">
	<div class="RequiredField">
		<h3><?= tpl_text('Address to import from') ?></h3>
		<input type="text" name="uri" value="<?= tpl_value($request['uri'],'http://www.synd.info/xmlrpc/language/') ?>" size="80" />
	</div>
	<div class="OptionalField">
		<?= tpl_form_checkbox('inclusive',$request['inclusive']) ?>
			<label for="inclusive"><?= tpl_text('Import strings not found in the local installation') ?></label><br />
		<?= tpl_form_checkbox('overwrite',$request['overwrite']) ?>
			<label for="overwrite"><?= tpl_text('Overwrite existing translations') ?></label>
	</div>
	<input type="submit" name="post" value="<?= tpl_text('Import') ?>" />
</form>