<?
if (null != $request['item'])
	$items = $node->_findItems($request['item']);
else if (null != $request['data']['item'])
	$items = array(SyndNodeLib::getInstance($request['data']['item']));

?>
<div class="Article">
	<div class="Header">
		<h2><?= tpl_text('Recieve item for repair') ?></h2>
		<div class="Info">
			 <?= tpl_text('Repair that does not require the client to have his/her equipment replaced. For example reinstalling the operating system.') ?>
		</div>
	</div>
	<? include tpl_design_path('gui/errors.tpl'); ?>

	<div class="RequiredField">
		<h3><?= tpl_text('Search for item or client') ?></h3>
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
						<td><?= tpl_form_radiobutton('data[item]', $request['data']['item'],$items[$key]->nodeId) ?></td>
						<td><? $this->render($items[$key],'list_view.tpl') ?></td>
					</tr>
					<? } ?>
				</table>
			<? } else { ?>
				<em><?= tpl_text("No results were found containing <b>'%s'</b>", $request['item']) ?></em>
			<? } ?>
		<? } ?>
	</div>

	<div class="RequiredField<? if (isset($errors['content'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Repair information') ?><? if (isset($errors['content'])) { ?> <span style="color:red;">(<?= $errors['content']['msg'] ?>)</span><? } ?></h3>
		<?= tpl_form_textarea('data[content]',$request['data']['content'],array('cols'=>53)) ?>
	</div>

	<div class="OptionalField">
		<?= tpl_form_checkbox('data[FLAG_NO_WARRANTY]', $data['FLAG_NO_WARRANTY']) ?>
			<label for="data[FLAG_NO_WARRANTY]"><?= tpl_text('Non warranty issue') ?></label>
	</div>
</div>

<span title="<?= tpl_text('Accesskey: %s', 'S') ?>">
	<input type="submit" accesskey="s" name="post" value="<?= tpl_text('Proceed >>>') ?>" />
</span>