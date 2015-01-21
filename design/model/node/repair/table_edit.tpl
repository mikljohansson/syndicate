<? 
tpl_sort_list($list, 'repair');
tpl_load_script(tpl_design_uri('js/form.js'));
?>
<table class="list" style="width:100%;" cellpadding="2">
	<thead>
		<tr>
			<th><a href="<?= tpl_sort_uri('issue','CLIENT_NODE_ID') ?>"><?= tpl_text('Client') ?></a></th>
			<th><a href="<?= tpl_sort_uri('issue','INFO_HEAD') ?>"><?= tpl_text('Description') ?></a></th>
		</tr>
	</thead>
	<tbody>
	<? foreach ($list as $node) { ?>
		<? $customer = $node->getCustomer(); ?>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td><a href="<?= tpl_link('user','summary',$customer->nodeId) ?>"><?= $customer->toString() ?></a></td>
			<td width="75%"><a href="<?= tpl_link($node->getHandler(),$node->objectId()) ?>"><?= $node->getTitle() ?></a></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<td><? $this->iterate($node->getItems(),'list_view.tpl') ?></td>
			<td>
				<? $id = "mplex[node/edit/{$node->nodeId}]"; ?>
				<input type="hidden" name="<?= $id ?>[post]" value="1" />
				<?= tpl_form_checkbox("{$id}[data][INFO_STATUS]",$node->isClosed(),synd_node_issue::CLOSED) ?>
					<label for="<?= $id ?>[data][INFO_STATUS]"><?= tpl_text('Items are repaired (resolve issue)') ?></label><br />
				<textarea name="<?= $id ?>[data][task][content]" cols="40" rows="2"
					depend="<?= $id ?>[data][INFO_STATUS]" match="\w+" message="<?= tpl_text('Please provide a repair description') ?>"></textarea>
			</td>
		</tr>
		<tr><td colspan="2">&nbsp;</td></tr>
	<? } ?>
	</tbody>
</table>
