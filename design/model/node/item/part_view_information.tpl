<? if (count($repairs = $node->getRepairs())) { ?>
	<h3><?= tpl_text('Repairs') ?></h3>
	<? $this->display('model/node/issue/table.tpl',array('list'=>$repairs)) ?>
<? } ?>

<? if (count($relations = $node->getClientRelations())) { ?>
	<h3><?= tpl_text('Customers') ?></h3>
	<? $this->iterate($relations,'list_view_client.tpl') ?>
<? } ?>

<? if (count($history = $node->getHistory())) { ?>
<h3><?= tpl_text('Notes and history') ?></h3>
<table>
	<thead>
		<tr>
			<th>&nbsp;</th>
			<th><?= tpl_text('Date') ?></th>
			<th><?= tpl_text('User') ?></th>
			<th>&nbsp;</th>
		</tr>
	</thead>
	<tbody>
	<? foreach (array_keys($history) as $key) { 
		$creator = $history[$key]->getCustomer(); ?>
	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<td><?= tpl_def($history[$key]->getDescription()) ?>
		<td class="Collapsed"><?= tpl_def(tpl_date('Y-m-d', $history[$key]->getCreatedTimestamp())) ?></td>
		<td class="Collapsed"><a href="<?= tpl_link('user','summary',$creator->nodeId) ?>"><?= $creator->toString() ?></a></td>
		<td class="Collapsed">
			<? if ($node->isPermitted('write')) { ?>
			<a href="<?= tpl_link_call($node->getHandler(),'delete',$history[$key]->nodeId) 
				?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" border="0" /></a>
			<? } else print '&nbsp;'; ?>
		</td>
	</tr>
	<? } ?>
	</tbody>
</table>
<? } ?>

<? if ($node->isPermitted('write')) { ?>
<form action="<?= tpl_view_call($node->getHandler(),'invoke',$node->nodeId,'addHistory') ?>" method="post">
	<h3><?= tpl_text('Attach note') ?></h3>
	<table>
		<tr>
			<td><textarea name="INFO_BODY" cols="40" rows="2" ></textarea></td>
		</tr>
		<tr>
			<td style="text-align:right;">
				<input type="submit" value="<?= tpl_text('Attach') ?>" />
			</td>
		</tr>
	</table>
</form>
<? } ?>