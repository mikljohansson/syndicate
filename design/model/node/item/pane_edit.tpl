<?
tpl_load_script(tpl_design_uri('js/json.js'));
tpl_load_script(tpl_design_uri('js/autocomplete.js'));
tpl_cycle(array('odd','even'));

?>
<table class="Vertical">
	<tr class="<?= tpl_cycle() ?><? if (isset($errors['PARENT_NODE_ID'])) print ' InvalidField'; ?>">
		<th><?= tpl_text('Folder') ?></th>
		<td>
			<? $inventory = Module::getInstance('inventory'); $folders = $inventory->getFolders(); ?>
			<select name="data[PARENT_NODE_ID]">
				<? $this->iterate(SyndLib::sort(SyndLib::filter($folders,'isPermitted','read')),'option_expand_children.tpl',
					array('selected' => $node->_storage->getInstance($data['PARENT_NODE_ID']))) ?>
			</select>
		</td>
	</tr>
	<? if (count($classes = $node->getClassOptions())) { ?>
	<tr class="<?= tpl_cycle() ?><? if (isset($errors['CLASS_NODE_ID'])) print ' InvalidField'; ?>">
		<th><?= tpl_text('Category') ?></th>
		<td>
			<select name="data[CLASS_NODE_ID]" onchange="this.form.submit();">
				<option value="">&nbsp;</option>
				<? $this->iterate($classes,'option.tpl',array('selected'=>$node->getClass())) ?>
			</select>
		</td>
	</tr>
	<? } ?>
	<tr class="<?= tpl_cycle() ?><? if (isset($errors['CLIENT_NODE_ID'])) print ' InvalidField'; ?>">
		<th><?= tpl_text('Customer') ?></th>
		<td>
			<? $client = $node->getCustomer(); ?>
			<? if (!($client instanceof synd_node_lease)) { ?>
				<input type="hidden" name="data[prevClient]" value="<?= tpl_attribute($data['client'],$client->objectId()) ?>" />
				<input type="text" size="50" name="data[client]" id="data[client]" value="<?= 
					tpl_value($data['client'],$client->objectId()) ?>" autocomplete="off" /></td>
			<? } else { ?>
				<? $this->render($node->getCustomer(),'contact.tpl') ?>
			<? } ?>
		</td>
	</tr>
	<tr class="<?= tpl_cycle() ?><? if (isset($errors['costcenter'])) print ' InvalidField'; ?>">
		<th><?= tpl_text('Costcenter') ?></th>
		<td><input type="text" size="50" name="data[costcenter]" id="data[costcenter]" value="<?= tpl_attribute($data['costcenter']) ?>" size="45" /></td>
	</tr>
	<tr class="<?= tpl_cycle() ?><? if (isset($errors['project'])) print ' InvalidField'; ?>">
		<th><?= tpl_text('Project') ?></th>
		<td><input type="text" size="50" name="data[project]" id="data[project]" value="<?= tpl_attribute($data['project']) ?>" size="45" /></td>
	</tr>
	<tr class="<?= tpl_cycle() ?><? if (isset($errors['INFO_LOCATION'])) print ' InvalidField'; ?>">
		<th><?= tpl_text('Location') ?></th>
		<td><input type="text" size="50" name="data[INFO_LOCATION]" value="<?= tpl_value($data['INFO_LOCATION']) ?>" /></td>
	</tr>
	<tr class="<?= tpl_cycle() ?><? if (isset($errors['INFO_MAKE'])) print ' InvalidField'; ?>">
		<th><?= tpl_text('Make') ?></th>
		<td><input type="text" size="50" name="data[INFO_MAKE]" value="<?= tpl_value($data['INFO_MAKE']) ?>" /></td>
	</tr>
	<tr class="<?= tpl_cycle() ?><? if (isset($errors['INFO_MODEL'])) print ' InvalidField'; ?>">
		<th><?= tpl_text('Model') ?></th>
		<td><input type="text" size="50" name="data[INFO_MODEL]" value="<?= tpl_value($data['INFO_MODEL']) ?>" /></td>
	</tr>
	<tr class="<?= tpl_cycle() ?><? if (isset($errors['INFO_SERIAL_INTERNAL'])) print ' InvalidField'; ?>">
		<th><?= tpl_text('Serial') ?></th>
		<td><input type="text" size="50" name="data[INFO_SERIAL_INTERNAL]" value="<?= tpl_value($data['INFO_SERIAL_INTERNAL']) ?>" /></td>
	</tr>
	<tr class="<?= tpl_cycle() ?><? if (isset($errors['INFO_SERIAL_MAKER'])) print ' InvalidField'; ?>">
		<th><?= tpl_text('Maker S/N') ?></th>
		<td><input type="text" size="50" name="data[INFO_SERIAL_MAKER]" value="<?= tpl_value($data['INFO_SERIAL_MAKER']) ?>" /></td>
	</tr>
	<tr class="<?= tpl_cycle() ?><? if (isset($errors['installation'])) print ' InvalidField'; ?>">
		<th><?= tpl_text('Installation ID') ?></th>
		<td><input type="text" size="50" name="data[installation]" value="<?= tpl_value($data['installation']) ?>" /></td>
	</tr>
	<tr class="<?= tpl_cycle() ?><? if (isset($errors['TS_DELIVERY'])) print ' InvalidField'; ?>">
		<th><?= tpl_text('Delivered') ?></th>
		<td><input type="text" size="50" name="data[TS_DELIVERY]" value="<?= tpl_date('Y-m-d',$node->data['TS_DELIVERY']) ?>" /></td>
	</tr>
	<tr class="<?= tpl_cycle() ?><? if (isset($errors['INFO_WARRANTY'])) print ' InvalidField'; ?>">
		<th><?= tpl_text('Warranty') ?></th>
		<td><input type="text" size="50" name="data[INFO_WARRANTY]" value="<?= tpl_value($data['INFO_WARRANTY']) ?>" /></td>
	</tr>
	<tr class="<?= tpl_cycle() ?><? if (isset($errors['INFO_COST'])) print ' InvalidField'; ?>">
		<th><?= tpl_text('Purchase value') ?></th>
		<td><input type="text" size="50" name="data[INFO_COST]" value="<?= tpl_value($data['INFO_COST']) ?>" /></td>
	</tr>
	<tr class="<?= tpl_cycle() ?><? if (isset($errors['INFO_RUNNING_COST'])) print ' InvalidField'; ?>">
		<th><?= tpl_text('Running cost') ?></th>
		<td><input type="text" size="50" name="data[INFO_RUNNING_COST]" value="<?= tpl_value($data['INFO_RUNNING_COST']) ?>" /></td>
	</tr>
	<? $class = $node->getClass(); if (!$class->isNull()) { 
		foreach (array_keys($fields = $class->getFields()) as $key) { ?>
		<tr class="<?= tpl_cycle() ?><? if (isset($errors[$key])) print ' InvalidField'; ?>">
			<th><?= $fields[$key]->toString() ?></th>
			<td><input type="text" size="50" name="data[values][<?= $key ?>]" value="<?= tpl_attribute($data['values'][$key]) ?>" /></td>
		</tr>
		<? } ?>
	<? } ?>
</table>

<script type="text/javascript">
<!--
	if (document.getElementById) {
		window.onload = function() {
			if (document.getElementById('data[client]'))
				new AutoComplete(document.getElementById('data[client]'), '<?= tpl_view('rpc','json',$node->id()) ?>', 'findSuggestedUsers', false);
			new AutoComplete(document.getElementById('data[costcenter]'), '<?= tpl_view('rpc','json','inventory') ?>', 'findSuggestedCostcenters', false);
			new AutoComplete(document.getElementById('data[project]'), '<?= tpl_view('rpc','json','inventory') ?>', 'findSuggestedProjects', false);
		};
	}
//-->
</script>
