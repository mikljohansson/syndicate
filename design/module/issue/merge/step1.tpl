<form method="post">
	<input type="hidden" name="collections[]" value="<?= $collection->id() ?>" />
	
	<div class="RequiredField">
		<h1><?= tpl_text('Select the primary issue') ?></h1>
	
		<table>
			<tr>
				<td><?= tpl_form_radiobutton('0',isset($request[0])?$request[0]:'new','new') ?></td>
				<td><label for="0[new]"><?= tpl_text('Create new primary issue') ?></label></td>
			</tr>
			<? foreach (array_keys($issues) as $key) { ?>
			<tr>
				<td><?= tpl_form_radiobutton('0',$request[0],$issues[$key]->nodeId) ?></td>
				<td><label for="0[<?= $issues[$key]->nodeId ?>]"><?= $issues[$key]->toString() ?></label></td>
			</tr>
			<? } ?>
		</table>
	</div>

	<div class="OptionalField">
		<?= tpl_form_checkbox('close',$data['close']) ?>
			<label for="close"><?= tpl_text('Mark child issues as closed') ?></label><br />
	</div>
	
	<p>
		<span title="<?= tpl_text('Accesskey: %s','S') ?>">
			<input accesskey="s" type="submit" class="button" value="<?= tpl_text('Ok') ?>" />
		</span>
		<span title="<?= tpl_text('Accesskey: %s','A') ?>">
			<input accesskey="a" type="button" class="button" value="<?= tpl_text('Abort') ?>"  onclick="window.location='<?= tpl_uri_return() ?>';" />
		</span>
	</p>
</form>
