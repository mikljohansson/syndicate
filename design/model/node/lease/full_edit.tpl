<? 
tpl_load_script(tpl_design_uri('js/json.js'));
tpl_load_script(tpl_design_uri('js/autocomplete.js'));
$inventory = Module::getInstance('inventory'); 
$tabindex1 = 1;
$tabindex2 = 8;

?>
<div class="Header">
	<h1><?= $node->toString() ?></h1>
</div>
<? if (!$node->isTerminated() || count($node->getItems())) { ?>
<ul class="Actions">
	<li><a href="<?= tpl_link_call('inventory','invoke',$node->nodeId,'terminate') ?>"><?= tpl_text('Terminate this lease') ?></a></li>
</ul>
<? } ?>
<? require tpl_design_path('gui/errors.tpl') ?>
<table class="Vertical">
	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<th><?= tpl_text('Customer') ?></th>
		<td><input type="text" name="data[customer]" id="data[customer]" value="<?= tpl_attribute($data['customer']) ?>" style="width:98%;" tabindex="<?= $tabindex1++ ?>" /></td>
		<th><?= tpl_text('Costcenter') ?></th>
		<td><input type="text" name="data[costcenter]" id="data[costcenter]" value="<?= tpl_attribute($data['costcenter']) ?>" style="width:98%;" tabindex="<?= $tabindex2++ ?>" /></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Folder') ?><? if (isset($errors['PARENT_NODE_ID'])) print '<span style="color:red;">*</span>'; ?></th>
		<td>
			<? $inventory = Module::getInstance('inventory'); $folders = $inventory->getFolders(); ?>
			<select name="data[PARENT_NODE_ID]" style="width:100%;" tabindex="<?= $tabindex1++ ?>">
				<? $this->iterate(SyndLib::sort(SyndLib::filter($folders,'isPermitted','read')),'option_expand_children.tpl',
					array('selected' => $node->_storage->getInstance($data['PARENT_NODE_ID']))) ?>
			</select>
		</td>
		<th><?= tpl_text('Project') ?></th>
		<td><input type="text" name="data[project]" id="data[project]" value="<?= tpl_attribute($data['project']) ?>" style="width:98%;" tabindex="<?= $tabindex2++ ?>" /></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Receipt') ?><? if (isset($errors['RECEIPT_NODE_ID'])) print '<span style="color:red;">*</span>'; ?></th>
		<td>
			<? $receipt = $node->getReceiptTemplate(); $receipts = $node->getReceiptOptions(); ?>
			<select name="data[RECEIPT_NODE_ID]" style="width:100%;" tabindex="<?= $tabindex1++ ?>">
				<? if (empty($receipts)) { ?>
				<option value="">&nbsp;</option>
				<? } ?>
				<? $this->iterate($receipts,'list_view_option.tpl',array('selected'=>$receipt)) ?>
			</select>
		</td>
		<th rowspan="6"><?= tpl_text('SLD') ?></th>
		<td rowspan="6">
			<? if (count($options = $node->getServiceLevelAgreementOptions())) { ?>
			<select name="data[sla]" tabindex="<?= $tabindex2++ ?>">
				<option value="">&nbsp;</option>
				<? $this->iterate(SyndLib::sort($options),'option.tpl') ?>
			</select>
			<input type="submit" value="<?= tpl_text('Add') ?>" tabindex="<?= $tabindex2++ ?>" />
			<? } ?>
			<? if (count($descriptions = $node->getServiceLevelDescriptions())) { ?>
			<table>
				<? foreach (array_keys(SyndLib::sort($descriptions)) as $key) { ?>
				<tr>
					<td><a href="<?= tpl_link($descriptions[$key]->getHandler(),'view',$descriptions[$key]->nodeId) 
						?>" title="<?= tpl_attribute(tpl_chop($descriptions[$key]->getDescription(),150)) ?>"><?= $descriptions[$key]->toString() ?></a></td>
					<td><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'delServiceLevelAgreement',$descriptions[$key]->nodeId) 
						?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= tpl_text('Remove') ?>" /></a></td>
				</tr>
				<? } ?>
			</table>
			<? } ?>
		</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Created') ?></th>
		<td><input type="text" name="data[created]" value="<?= tpl_date('Y-m-d', $node->data['TS_CREATE']) ?>" tabindex="<?= $tabindex1++ ?>" /> (YYYY-MM-DD)</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Expires') ?></th>
		<td><input type="text" name="data[expires]" value="<?= tpl_date('Y-m-d', $node->data['TS_EXPIRE']) ?>" tabindex="<?= $tabindex1++ ?>" /> (YYYY-MM-DD)</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Terminated') ?></th>
		<td><input type="text" name="data[terminated]" value="<?= tpl_date('Y-m-d', $node->data['TS_TERMINATED']) ?>" tabindex="<?= $tabindex1++ ?>" /> (YYYY-MM-DD)</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Notes') ?></th>
		<td><?= tpl_form_textarea('data[INFO_BODY]',$data['INFO_BODY'],array('cols'=>40,'rows'=>5,'tabindex'=>$tabindex1++)) ?></td>
	</tr>
</table>

<p>
	<span title="<?= tpl_text('Accesskey: %s','S') ?>">
		<input accesskey="s" type="submit" name="post" value="<?= tpl_text('Save') ?>" tabindex="<?= $tabindex2++ ?>" />
	</span>
	<span title="<?= tpl_text('Accesskey: %s','A') ?>">
		<input accesskey="a" type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>'" tabindex="<?= $tabindex2++ ?>" />
	</span>
	<? if (!$node->isNew()) { ?>
	<input type="button" value="<?= tpl_text('Delete') ?>" onclick="window.location='<?= tpl_view('inventory','delete',$node->nodeId) ?>'" tabindex="<?= $tabindex2++ ?>" />
	<? } ?>
</p>

<script type="text/javascript">
<!--
	if (document.getElementById) {
		window.onload = function() {
			new AutoComplete(document.getElementById('data[customer]'), '<?= tpl_view('rpc','json',$node->id()) ?>', 'findSuggestedUsers');
			new AutoComplete(document.getElementById('data[costcenter]'), '<?= tpl_view('rpc','json',$node->id()) ?>', 'findSuggestedCostcenters');
			new AutoComplete(document.getElementById('data[project]'), '<?= tpl_view('rpc','json',$node->id()) ?>', 'findSuggestedProjects');
		};
	}
//-->
</script>
