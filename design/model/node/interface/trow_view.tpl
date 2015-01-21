<? 
$config = $node->getParent();
$device = $config->getParent();
$customer = $device->getCustomer();
$nic = $node->getNetworkInterface();
?>
		<tr class="<?= tpl_cycle(array('odd','even')) ?> <?= $nic->isNull()?'Unknown':$nic->getStatusName() ?>">
			<td width="10">
				<? if ($device->isPermitted('write')) { ?>
				<a href="<?= tpl_link_call('inventory','edit',$device->nodeId,'config') ?>"><img src="<?= tpl_design_uri('image/icon/record.gif') ?>" /></a>
				<? } else print '&nbsp;'; ?>
			</td>
			<td><a href="<?= tpl_link('inventory','view',$device->nodeId,'config') ?>"><?= tpl_def($node->getIpAddress()) ?></a></td>
			<td><a href="<?= tpl_link('inventory','view',$device->nodeId,'config') ?>"><?= tpl_def($node->getHostname()) ?></a></td>
			<td><a href="<?= tpl_link('inventory','view',$device->nodeId,'config') ?>"><?= $nic->isNull() ? '&nbsp;' : tpl_def($nic->getMacAddress()) ?></a></td>
			<td><a href="<?= tpl_link('inventory','view',$device->nodeId) ?>"><?= tpl_default($device->getTitle(), tpl_text('Unknown')) ?></a></td>
			<td><a href="<?= tpl_link('inventory','view',$device->nodeId) ?>"><?= $device->data['INFO_SERIAL_MAKER'] ?></a></td>
			<td><a href="<?= tpl_link('user','summary',$customer->nodeId) ?>"><?= $customer->toString() ?></a></td>
			<td class="Status" title="<?= tpl_strftime('%Y-%m-%d %H:%I:%S %Z',$nic->getLastSeen()) ?>"><div><img src="<?= tpl_design_uri('image/pixel.gif') ?>" alt="" /></div></td>
			<? if (empty($hideCheckbox)) { ?>
			<td class="OLE" onmouseover="this.parentNode.setAttribute('_checked',true);" onmouseout="this.parentNode.setAttribute('_checked',this.firstChild.checked);"><input type="checkbox" name="selection[]" value="<?= $device->id() ?>" /></td>
			<? } ?>
		</tr>
