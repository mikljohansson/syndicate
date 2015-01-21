<?
$item = $node->getItem();
$items = array($item->nodeId => $item); 
if (null != $request['query'])
	$items = SyndLib::array_merge_assoc($items, $node->_findItems($request['query']));

if (isset($request['data']))
	$selected = $request['data']['item'];
else
	$selected = $data['CHILD_NODE_ID'];

?>
<div class="Article">
	<? $this->render($node,'list_view.tpl') ?>
	<? include tpl_design_path('gui/errors.tpl'); ?>

	<h3><?= tpl_text('Returned date (YYYY-MM-DD)') ?></h3>
	<div class="indent">
		<input type="text" name="data[TS_EXPIRE]" value="<?= tpl_date('Y-m-d', $node->data['TS_EXPIRE']) ?>" />
	</div>

	<br />
	<h3><?= tpl_text('Search for and set item') ?></h3>

	<div class="indent">
		<input type="text" name="query" value="<?= tpl_value($request['query']) ?>" size="71" />
		<input type="submit" value="<?= tpl_text('Search') ?>" />

		<br />
		<table>
			<? if (isset($items)) { ?>
				<? foreach (array_keys($items) as $key) { ?>
				<tr>
					<? $client = $items[$key]->getCustomer(); if (!$client->isPermitted('remove',$items[$key]) && $items[$key]->nodeId != $item->nodeId) { ?>
					<td><input type="radio" disabled="disabled" /></td>
					<? } else { ?>
					<td><?= tpl_form_radiobutton('data[item]',$selected,$items[$key]->nodeId) ?></td>
					<? } ?>
					<td><? $this->render($items[$key],'list_view.tpl') ?></td>
				</tr>
				<? } ?>
			<? } ?>
		</table>
	</div>

	<input type="submit" name="post" value="<?= tpl_text('Save') ?>" />
	<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
	<input type="button" value="<?= tpl_text('Delete') 
		?>" onclick="window.location='<?= tpl_view_jump($node->getHandler(),'delete',$node->nodeId) ?>';" />
</div>
