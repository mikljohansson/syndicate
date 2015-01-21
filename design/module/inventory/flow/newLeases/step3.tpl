<form method="post">
	<h2><?= tpl_text('Batch create new leases') ?></h2>
	<div class="indent">
		Please select the users that you wish to create a new lease for.
	</div>
	<br />

	<h3><?= tpl_text('Prototype lease') ?></h3>

	<div class="indent">

		<? $this->render($prototype,'list_view_duration.tpl') ?>
	</div>
	<br />

	<? if (count($clients)) { ?>
		<? $clients = SyndLib::sort($clients); ?>
		<table>
			<? foreach (array_keys($clients) as $key) { ?>
			<tr>
				<td><input type="checkbox" name="clients[]" 
					value="<?= $clients[$key]->nodeId ?>" checked="checked" /></td>
				<td>
					<? $this->render($clients[$key],'head_view.tpl') ?>
					<? $this->render($clients[$key],'head_view_contact.tpl') ?>
				</td>
			</tr>
			<? } ?>
		</table>
		<br />
		<input type="submit" value="<?= tpl_text('Confirm') ?>" />
		
	<? } else { ?>
		<em><?= tpl_text("No results were found") ?></em>
	<? } ?>
</form>