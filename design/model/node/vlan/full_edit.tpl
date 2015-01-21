<div class="Article">
	<h1><?= $node->isNew() ? tpl_text('New VLAN') : $node->toString() ?></h1>
	<? include tpl_design_path('gui/errors.tpl'); ?>
	
	<div class="RequiredField<? if (isset($errors['INFO_HEAD'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Name of VLAN') ?></h3>
		<input type="text" name="data[INFO_HEAD]" value="<?= tpl_attribute($data['INFO_HEAD']) ?>" />
	</div>

	<div class="RequiredField<? if(isset($errors['PARENT_NODE_ID'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Parent folder') ?></h3>
		<select name="data[PARENT_NODE_ID]">
			<? $this->display(tpl_design_path('module/inventory/folder_options.tpl'),
				array('selected' => SyndNodeLib::getInstance($data['PARENT_NODE_ID']))); ?>
		</select>
	</div>
	
	<div class="OptionalField<? if (isset($errors['INFO_DESC'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Description and usage instructions') ?></h3>
		<?= tpl_form_textarea('data[INFO_DESC]',$data['INFO_DESC'],array('cols'=>50)) ?>
	</div>

	<div class="OptionalField<? if (isset($errors['network'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Networks assigned to this VLAN') ?></h3>
		<table>
			<thead>
				<tr>
					<th><?= tpl_text('IP-address') ?></td>
					<th><?= tpl_text('Netmask') ?></th>
					<th>&nbsp;</th>
				</tr>
			</thead>
			<tbody>
				<? foreach (array_keys($networks = $node->getNetworks()) as $key) { ?>
				<tr class="<?= tpl_cycle(array('odd','even')) ?>">
					<td><?= $networks[$key]['INFO_NETWORK_ADDRESS'] ?></td>
					<td><?= $networks[$key]['INFO_NETWORK_MASK'] ?></td>
					<td><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'delNetwork',$networks[$key]['NODE_ID']) 
						?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= tpl_text('Delete') ?>" /></a></td>
				</tr>
				<? } ?>
			</tbody>
			<tfoot>
				<tr class="<?= tpl_cycle(array('odd','even')) ?>">
					<td><input type="text" name="data[network][INFO_NETWORK_ADDRESS]" size="15" /></td>
					<td><input type="text" name="data[network][INFO_NETWORK_MASK]" size="15" /></td>
					<td><input type="submit" value="<?= tpl_text('Add') ?>" /></td>
				</tr>
			</tfoot>
		</table>
	</div>
	<p>
		<span title="<?= tpl_text('Accesskey: %s','S') ?>">
			<input accesskey="s" type="submit" name="post" value="<?= tpl_text('Save') ?>" />
		</span>
		<span title="<?= tpl_text('Accesskey: %s','A') ?>">
			<input accesskey="a" type="button" value="<?= tpl_text('Abort') ?>"  onclick="window.location='<?= tpl_uri_return() ?>';" />
		</span>
		<? if (!$node->isNew()) { ?>
		<input class="button" type="button" value="<?= tpl_text('Delete') ?>" onclick="window.location='<?= tpl_view('node','delete',$node->nodeId) ?>';" />
		<? } ?>
	</p>
</div>