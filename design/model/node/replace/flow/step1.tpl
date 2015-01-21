<?
if (null != $request['item'])
	$items = $node->_findItems($request['item']);
else if (null != $request['data']['item'])
	$items = array(SyndNodeLib::getInstance($request['data']['item']));

?>
<div class="Article">
	<div class="Header">
		<h2><?= tpl_text('Replacement of faulty item') ?></h2>
		<div class="Info">
			<?= tpl_text('Extended repair that requires equipment to be shipped off to be repaired. The client will in most cases have her items replaced.') ?>
		</div>
	</div>
	<? include tpl_design_path('gui/errors.tpl'); ?>

	<div class="RequiredField">
		<h3><?= tpl_text('Item to recieve for repair') ?></h3>
		<input type="text" name="item" value="<?= tpl_value($request['item']) ?>" size="71" /><br />
		<? if (isset($items)) { ?>
			<? if (count($items)) { ?>
			<table>
				<tr>
					<td><?= tpl_form_radiobutton('data[item]',$request['data']['item'],'','SearchItemAgain') ?></td>
					<td><label for="SearchItemAgain"><?= tpl_text('Search again') ?></label></td>
				</tr>
				<? foreach (array_keys($items) as $key) { ?>
				<tr>
					<? $client = $items[$key]->getCustomer(); if ($client->isPermitted('remove',$items[$key])) { ?>
						<td><input type="radio" disabled="disabled" /></td>
					<? } else { ?>
						<td><?= tpl_form_radiobutton('data[item]', 
								$request['data']['item'],$items[$key]->nodeId) ?></td>
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

	<div class="RequiredField">
		<h3><?= tpl_text('Replacement item to give client') ?></h3>
		<input type="text" name="replacement" value="<?= tpl_value($request['replacement']) ?>" size="71" /><br />
		<? if (null != $request['replacement']) { ?>
			<? if (count($matches = $node->_findItems($request['replacement']))) { ?>
			<table>
				<tr>
					<td><?= tpl_form_radiobutton('data[replacement]',$request['data']['replacement'],'','SearchReplacementAgain') ?></td>
					<td><label for="SearchReplacementAgain"><?= tpl_text('Search again') ?></label></td>
				</tr>
				<? foreach (array_keys($matches) as $key) { ?>
				<tr>
					<? $client = $matches[$key]->getCustomer(); if ($client->isPermitted('remove',$matches[$key])) { ?>
						<td><?= tpl_form_radiobutton('data[replacement]', 
								$request['data']['replacement'],$matches[$key]->nodeId) ?></td>
					<? } else { ?>
						<td><input type="radio" disabled="disabled" /></td>
					<? } ?>
					<td><? $this->render($matches[$key],'list_view.tpl') ?></td>
				</tr>
				<? } ?>
			</table>
			<? } else { ?>
				<em><?= tpl_text("No results were found containing <b>'%s'</b>", $request['replacement']) ?></em>
			<? } ?>
		<? } ?>
	</div>

	<div class="RequiredField<? if (isset($errors['content'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Repair information') ?></h3>
		<?= tpl_form_textarea('data[content]',$request['data']['content'],array('cols'=>53)) ?>
	</div>

	<div class="OptionalField">
		<?= tpl_form_checkbox('data[FLAG_NO_WARRANTY]', $data['FLAG_NO_WARRANTY']) ?>
		<label for="data[FLAG_NO_WARRANTY]"><?= tpl_text('Non warranty issue') ?></label>
	</div>

	<span title="<?= tpl_text('Accesskey: %s', 'S') ?>">
		<input type="submit" accesskey="s" name="post" value="<?= tpl_text('Proceed >>>') ?>" />
	</span>
</div>
