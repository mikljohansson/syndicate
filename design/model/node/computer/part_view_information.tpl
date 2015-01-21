<? 
$seen = array(); 
$nics = $node->getNetworkInterfaces();
$config = $node->getConfig(); 
$interfaces = $config->getInterfaces();

foreach (array_keys($interfaces) as $key) {
	$setparts = explode('.', $interfaces[$key]->getHostname());
	$realhost = gethostbyaddr($interfaces[$key]->getIpAddress());
	$realparts = explode('.', $realhost);
	
	if (null != $interfaces[$key]->getHostname() && 
		null != $interfaces[$key]->getIpAddress() && 
		strtolower(reset($setparts)) != strtolower(reset($realparts)))
		$errors[] = tpl_text("DNS mismatch on <em>'%s'</em>, hostname in DNS is <em>'%s'</em>", $interfaces[$key]->getHostname(), $host);
}

?>
<? if (!empty($nics) || !empty($interfaces)) { ?>
<h3><?= tpl_text('Network interfaces') ?></h3>
<? include tpl_design_path('gui/errors.tpl'); ?>
<table class="Interfaces">
	<thead>
		<tr>
			<th><?= tpl_text('Hostname') ?></td>
			<th><?= tpl_text('IP address') ?></td>
			<th><?= tpl_text('VLAN') ?></td>
			<th><?= tpl_text('MAC address') ?></td>
			<th><?= tpl_text('Switch') ?></td>
			<th><?= tpl_text('Port') ?></td>
			<th><?= tpl_text('Description') ?></th>
			<th>&nbsp;</th>
		</tr>
	</thead>
	<tbody>
		<? foreach (array_keys($nics) as $key) { 
			$interface = $nics[$key]->getInterface();
			$seen[] = $interface->nodeId; ?>
		<tr class="<?= tpl_cycle(array('odd','even')) ?> <?= $nics[$key]->getStatusName() ?>">
			<td><?= tpl_def($interface->isNull() ? '' : $interface->getHostname()) ?></td>
			<td><?= tpl_def($interface->isNull() ? '' : $interface->getIpAddress()) ?></td>
			<td>
				<? if (!$interface->isNull() && null !== ($vlan = $interface->getVirtualLan()) && !$vlan->isNull()) { ?>
				<a href="<?= tpl_link('inventory','view',$vlan->nodeId) ?>"><?= $vlan->toString() ?></a>
				<? } else print '&nbsp;'; ?>
			</td>
			<td><?= tpl_def($nics[$key]->getMacAddress()) ?></td>
			<td title="<?= @gethostbyaddr($nics[$key]->getLastSwitch()) ?>"><?= tpl_def($nics[$key]->getLastSwitch()) ?></td>
			<td><?= tpl_def($nics[$key]->getLastSwitchPort()) ?></td>
			<td><?= tpl_def($nics[$key]->getDescription()) ?></td>
			<td class="Status" title="<?= tpl_strftime('%Y-%m-%d %H:%I:%S %Z',$nics[$key]->getLastSeen()) ?>"><div><img src="<?= tpl_design_uri('image/pixel.gif') ?>" alt="" /></div></td>
		</tr>
		<? } ?>
		<? foreach (array_keys($interfaces) as $key) { ?>
		<? if (in_array($interfaces[$key]->nodeId, $seen)) continue; ?>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td><?= tpl_def($interfaces[$key]->getHostname()) ?></td>
			<td><?= tpl_def($interfaces[$key]->getIpAddress()) ?></td>
			<td>&nbsp;</td>
			<td>&nbsp;</td>
		</tr>
		<? } ?>
	</tbody>
</table>
<? } ?>

<? include $this->path('synd_node_item','part_view_information.tpl'); ?>