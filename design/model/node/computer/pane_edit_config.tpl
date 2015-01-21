<? 
$seen = array();
$nics = $node->getNetworkInterfaces(); 
$config = $node->getConfig();
$interfaces = $config->getInterfaces(); 
$vlans = SyndLib::sort(SyndLib::filter($config->getVirtualLans(), 'isPermitted', 'write'), 'toString');
tpl_load_script(tpl_design_uri('js/json.js')); 

?>
<h3><?= tpl_text('Network interface cards') ?></h3>
<table>
	<thead>
		<tr>
			<th><?= tpl_text('MAC address') ?></td>
			<th><?= tpl_text('Description') ?></th>
			<th>&nbsp;</th>
		</tr>
	</thead>
	<tbody>
		<? foreach (array_keys($nics) as $key) { ?>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td><input type="text" name="data[nics][<?= $nics[$key]->nodeId ?>][INFO_MAC_ADDRESS]" value="<?= 
				tpl_attribute($data['nics'][$nics[$key]->nodeId]['INFO_MAC_ADDRESS']) ?>" size="20" /></td>
			<td><input type="text" name="data[nics][<?= $nics[$key]->nodeId ?>][INFO_HEAD]" value="<?= 
				tpl_attribute($data['nics'][$nics[$key]->nodeId]['INFO_HEAD']) ?>" size="60" /></td>
			<td><a href="<?= tpl_link_call($nics[$key]->getHandler(),'delete',$nics[$key]->nodeId) 
				?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= tpl_text('Delete') ?>" /></a></td>
		</tr>
		<? } ?>
	</tbody>
	<tfoot>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td><input type="text" name="data[nics][0][INFO_MAC_ADDRESS]" size="20" /></td>
			<td><input type="text" name="data[nics][0][INFO_HEAD]" size="60" /></td>
			<td><input type="submit" value="<?= tpl_text('Add') ?>" /></td>
		</tr>
	</tfoot>
</table>

<h3><?= tpl_text('Configured interfaces') ?></h3>
<table>
	<thead>
		<tr>
			<th><?= tpl_text('Hostname') ?></td>
			<th><?= tpl_text('IP-address') ?></th>
			<? if (!empty($vlans)) { ?>
			<th><?= tpl_text('VLAN') ?></th>
			<? } ?>
			<th><?= tpl_text('MAC-address') ?></th>
			<th>&nbsp;</th>
		</tr>
	</thead>
	<tbody>
		<? foreach (array_keys($interfaces) as $key) { 
			$vlan = $interfaces[$key]->getVirtualLan(); ?>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td><input type="text" name="data[interfaces][<?= $interfaces[$key]->nodeId ?>][INFO_HOSTNAME]" value="<?= 
				tpl_attribute($data['interfaces'][$interfaces[$key]->nodeId]['INFO_HOSTNAME']) ?>" size="20" /></td>
			<td><input type="text" name="data[interfaces][<?= $interfaces[$key]->nodeId ?>][INFO_IP_ADDRESS]" value="<?= 
				tpl_attribute($data['interfaces'][$interfaces[$key]->nodeId]['INFO_IP_ADDRESS']) ?>" size="15" id="<?= $interfaces[$key]->nodeId ?>[ip]" /></td>
			<? if (!empty($vlans)) { ?>
			<td>
				<select id="<?= $interfaces[$key]->nodeId ?>" onchange="_callback_vlan_onchange(this);" onkeyup="_callback_vlan_onchange(this);" onmousewheel="return false;">
					<option value="">&nbsp;</option>
					<?= tpl_form_options(SyndLib::invoke($vlans,'toString'),$vlan->nodeId) ?>
				</select>
			</td>
			<? } ?>
			<td>
				<? $nic = $interfaces[$key]->getNetworkInterface(); if (!$nic->isNull()) $seen[] = $nic->getMacAddress(); ?>
				<select name="data[interfaces][<?= $interfaces[$key]->nodeId ?>][NIC_NODE_ID]">
					<option value="">&nbsp;</option>
					<?= tpl_form_options(SyndLib::invoke($node->getNetworkInterfaces(),'getMacAddress'),$nic->nodeId) ?>
				</select>
			</td>
			<td><a href="<?= tpl_link_call($interfaces[$key]->getHandler(),'delete',$interfaces[$key]->nodeId) 
				?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= tpl_text('Delete') ?>" /></a></td>
		</tr>
		<? } ?>
	</tbody>
	<tfoot>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td><input type="text" name="data[interfaces][0][INFO_HOSTNAME]" size="20" /></td>
			<td><input type="text" name="data[interfaces][0][INFO_IP_ADDRESS]" size="15" id="interface.0[ip]" /></td>
			<? if (!empty($vlans)) { ?>
			<td>
				<select id="interface.0" onchange="_callback_vlan_onchange(this);" onkeyup="_callback_vlan_onchange(this);" onmousewheel="return false;">
					<option value="">&nbsp;</option>
					<?= tpl_form_options(SyndLib::invoke($vlans,'toString')) ?>
				</select>
			</td>
			<? } ?>
			<td>
				<select name="data[interfaces][0][NIC_NODE_ID]">
					<?= tpl_form_options(array_diff(SyndLib::invoke($nics,'getMacAddress'),$seen)) ?>
				</select>
			</td>
			<td><input type="submit" value="<?= tpl_text('Add') ?>" /></td>
		</tr>
	</tfoot>
</table>

<script type="text/javascript">
<!--
	function _callback_vlan_onchange(oSelect) {
		if (document.getElementById) {
			try {
				var oTransport = new JsonTransport('<?= tpl_view('rpc','json') ?>node.'+oSelect.value+'/'), sValue = oSelect.value;
				oTransport.invoke('getAvailableIpAddress', new Array(oSelect.id), function(sIpAddress) {
						if (sValue == oSelect.value) {
							var oIpInput = document.getElementById(oSelect.id+'[ip]');
							if ('' == sIpAddress)
								alert('<?= tpl_text('No unallocated addresses available on this VLAN') ?>');
							else if (null != oIpInput)
								oIpInput.value = sIpAddress;
						}
					});
			}
			catch (e) {
				if (!(e instanceof SyndUnsupportedException))
					throw e;
			}
		}
	}
//-->
</script>
