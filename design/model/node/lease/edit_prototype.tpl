<input type="hidden" name="stack[]" value="<?= tpl_view('inventory','newLeases',array('prototype' => $node->nodeId)) ?>" />
<input type="hidden" name="data[CLIENT_NODE_ID]" value="user_null.null" />

<h1><?= tpl_text('Batch create new leases') ?></h1>
<table class="Article">
	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<th>
			<?= tpl_text('Folder') ?>
			<? if (isset($errors['PARENT_NODE_ID'])) print '<span style="color:red;">*</span>'; ?>
		</th>
		<td>
			<select name="data[PARENT_NODE_ID]">
				<? $this->iterate($node->getFolderOptions(),'list_view_option.tpl',
					array('selected' => $node->getFolder())) ?>
			</select>
		</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Created') ?></th>
		<td><input type="text" name="data[created]" value="<?= tpl_date('Y-m-d', $node->data['TS_CREATE']) ?>" /> (YYYY-MM-DD)</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Expires') ?></th>
		<td><input type="text" name="data[expires]" value="<?= tpl_date('Y-m-d', $node->data['TS_EXPIRE']) ?>" /> (YYYY-MM-DD)</td>
	</tr>
</table>
<br />

<input type="submit" name="post" value="<?= tpl_text('Proceed') ?> >>>" />