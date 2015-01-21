<form method="post">
	<div class="Article">
		<div class="Header">
			<h2><?= tpl_text('Hand out item on a lease') ?></h2>
			<div class="Info">
				<?= tpl_text('Append an item to an existing lease.') ?>
			</div>
		</div>

		<div class="RequiredField">
			<h3><?= tpl_text('Search for a client lease') ?></h3>
			<input type="text" name="lease" value="<?= tpl_value($request['lease']) ?>" size="71" /><br />
			<? if (isset($leases)) { ?>
				<? if (count($leases)) { ?>
				<table>
					<tr>
						<td><?= tpl_form_radiobutton('lease_node_id',$request['lease_node_id'],'','SearchLeaseAgain') ?></td>
						<td><label for="SearchLeaseAgain"><?= tpl_text('Search again') ?></label></td>
					</tr>
					<? foreach (array_keys($leases) as $key) { ?>
					<tr>
						<? if ($leases[$key]->isPermitted('append')) { ?>
							<td><?= tpl_form_radiobutton('lease_node_id',isset($lease)?$lease->nodeId:null,$leases[$key]->nodeId) ?></td>
						<? } else { ?>
							<td><input type="radio" disabled="disabled" /></td>
						<? } ?>
						<td><? $this->render($leases[$key],'list_view.tpl') ?></td>
					</tr>
					<? } ?>
				</table>
				<? } else { ?>
					<em><?= tpl_text("No results were found containing <b>'%s'</b>", $request['lease']) ?></em>
				<? } ?>
			<? } ?>
		</div>

		<div class="RequiredField">
			<h3><?= tpl_text('Search for an item') ?></h3>
			<input type="text" name="item" value="<?= tpl_value($request['item']) ?>" size="71" /><br />
			<? if (isset($items)) { ?>
				<? if (count($items)) { ?>
				<table>
					<tr>
						<td><?= tpl_form_radiobutton('item_node_id',$request['item_node_id'],'','SearchItemAgain') ?></td>
						<td><label for="SearchItemAgain"><?= tpl_text('Search again') ?></label></td>
					</tr>
					<? foreach (array_keys($items) as $key) { ?>
					<tr>
						<? $client = $items[$key]->getCustomer(); if ($client->isPermitted('remove',$items[$key])) { ?>
							<td><?= tpl_form_radiobutton('item_node_id',isset($item)?$item->nodeId:null,$items[$key]->nodeId) ?></td>
						<? } else { ?>
							<td><input type="radio" disabled="disabled" /></td>
						<? } ?>
						<td><? $this->render($items[$key],'list_view.tpl') ?></td>
					</tr>
					<? } ?>
				</table>
				<? } else { ?>
					<em><?= tpl_text("No results were found containing <b>'%s'</b>", $request['item']) ?></em>
				<? } ?>
			<? } ?>
		</div>

		<input type="submit" name="post" value="<?= tpl_text('Proceed >>>') ?>" />
	</div>
</form>