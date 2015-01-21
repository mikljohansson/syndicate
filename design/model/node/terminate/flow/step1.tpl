<form method="post">
	<div class="Article">
		<div class="Header">
			<h2><?= tpl_text('Terminate a lease') ?></h2>
			<div class="Info">
				<?= tpl_text('Terminate a client lease and make sure all leased items are returned.') ?>
			</div>
		</div>

		<h3><?= tpl_text('Search for a client lease') ?></h3>
		<div class="RequiredField">
			<input type="text" name="lease" value="<?= tpl_value($request['lease']) ?>" size="71" /><br />
			<? if (null != $request['lease']) { ?>
				<? if (count($leases = $node->_findLeases($request['lease']))) { ?>
				<table>
					<tr>
						<td><?= tpl_form_radiobutton('data[CLIENT_NODE_ID]',$request['data']['CLIENT_NODE_ID'],'','SearchLeaseAgain') ?></td>
						<td><label for="SearchLeaseAgain"><?= tpl_text('Search again') ?></label></td>
					</tr>
					<? foreach (array_keys($leases) as $key) { ?>
					<tr>
						<td><?= tpl_form_radiobutton('data[CLIENT_NODE_ID]', 
							$request['data']['CLIENT_NODE_ID'],$leases[$key]->nodeId) ?></td>
						<td><? $this->render($leases[$key],'list_view.tpl') ?></td>
					</tr>
					<? } ?>
				</table>
				<? } else { ?>
					<em><?= tpl_text("No results were found containing <b>'%s'</b>", $request['lease']) ?></em>
				<? } ?>
			<? } ?>
		</div>
		
		<input type="submit" name="post" value="<?= tpl_text('Proceed >>>') ?>" />
	</div>
</form>