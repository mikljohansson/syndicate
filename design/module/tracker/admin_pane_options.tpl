<? if (!empty($failed)) { ?>
<ul class="Warning">
	<? foreach ($failed as $urn) { ?>
	<li><?= tpl_text("Host '%s' is marked as failed.", $urn) ?> <? if (count($stats) > count($failed)) { ?><?= tpl_text('Load is being distributed to other servers.') ?><? } else { ?><?= tpl_text('No other servers to failover to, storage is inoperable.') ?><? } ?></li>
	<? } ?>
</ul>
<? } ?>

<form action="<?= tpl_link_call('tracker','saveDevices') ?>" method="post">
	<table class="Report">
		<? foreach ((array)$namespaces as $namespace) { ?>
		<? if ($i++) { ?>
		<tr><td colspan="6">&nbsp;</td></tr>
		<? } ?>
		<tr>
			<th>&nbsp;</th>
			<th colspan="3"><?= tpl_text('Stored LOBs') ?></th>
			<th style="border-left:1px solid;" colspan="3"><?= tpl_text('Logical usage') ?></th>
			<th style="border-left:1px solid;" colspan="5"><?= tpl_text('Physical usage') ?></th>
		</tr>
		<tr>
			<th>&nbsp;</th>
			<th class="Numeric"><?= tpl_text('Active') ?></th>
			<th class="Numeric"><?= tpl_text('Unreplicated') ?></th>
			<th class="Numeric"><?= tpl_text('Deleted') ?></th>
			<th class="Numeric" style="border-left:1px solid;"><?= tpl_text('Used') ?></th>
			<th class="Numeric"><?= tpl_text('Free') ?></th>
			<th class="Numeric"><?= tpl_text('Total') ?></th>
			<th class="Numeric" style="border-left:1px solid;"><?= tpl_text('Used') ?></th>
			<th class="Numeric"><?= tpl_text('Free') ?></th>
			<th class="Numeric"><?= tpl_text('Total') ?></th>
			<th>&nbsp;</th>
			<th>&nbsp;</th>
		</tr>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<th><?= $namespace['NAMESPACE'] ?></th>
			<td class="Numeric"><?= (float)$namespace['LOBCOUNT'] ?></td>
			<td class="Numeric"><?= (float)$namespace['REPCOUNT'] ?></td>
			<td class="Numeric"><?= (float)$namespace['DELCOUNT'] ?></td>
			<td class="Numeric" style="border-left:1px solid;"><?= round($namespace['LOBSPACE']/1024/1024) ?>Mb</td>
			<td class="Numeric">&nbsp;</td>
			<td class="Numeric">&nbsp;</td>
			<td class="Numeric" style="border-left:1px solid;">&nbsp;</td>
			<td class="Numeric">&nbsp;</td>
			<td class="Numeric">&nbsp;</td>
			<td class="Numeric">&nbsp;</td>
			<td class="Numeric">&nbsp;</td>
		</tr>
			<? foreach ((array)$devices as $device) { 
				if ($device['NSID'] != $namespace['NSID']) continue; ?>
			<tr class="<?= tpl_cycle() ?>">
				<td><? $this->display(tpl_design_path('module/tracker/urn.tpl'),array('urn'=>$device['URN'])) ?></td>
				<td class="Numeric"><?= (float)$device['LOBCOUNT'] ?></td>
				<td class="Numeric">&nbsp;</td>
				<td class="Numeric"><?= (float)$device['DELCOUNT'] ?></td>
				<td class="Numeric" style="border-left:1px solid;"><?= round($device['LOBSPACE']/1024/1024) ?>Mb</td>
				<td class="Numeric"><?= round(($device['SPACE']-$device['LOBSPACE'])/1024/1024) ?>Mb</td>
				<td class="Numeric">
					<input type="text" name="devices[<?= $device['DEVID'] ?>]" value="<?= round($device['SPACE']/1024/1024) ?>" size="4" />
				</td>
				<td class="Numeric" style="border-left:1px solid;"><?= round(($stats[$device['URN']]['total'] - $stats[$device['URN']]['free'])/1024/1024) ?>Mb</td>
				<td class="Numeric"><?= round($stats[$device['URN']]['free']/1024/1024) ?>Mb</td>
				<td class="Numeric"><?= round($stats[$device['URN']]['total']/1024/1024) ?>Mb</td>
				<td>
					<? if (!is_numeric($device['FAILED'])) { ?>
					<img src="<?= tpl_design_uri('image/icon/16x16/offline.gif') ?>" alt="<?= tpl_text('Offline') 
						?>" title="<?= tpl_text('Device has been taken offline') ?>" width="16" height="16" />
					<? } else if ($device['FAILED'] >= time()-$module->_retryInterval) { ?>
					<img src="<?= tpl_design_uri('image/icon/16x16/failed.gif') ?>" alt="<?= tpl_text('Failed') 
						?>" title="<?= tpl_text('Device is temporarily marked as failed') ?>" width="16" height="16" />
					<? } else { ?>
					<img src="<?= tpl_design_uri('image/icon/16x16/online.gif') ?>" alt="<?= tpl_text('Online') 
						?>" title="<?= tpl_text('Device is fully functional') ?>" width="16" height="16" />
					<? } ?>
				</td>
				<td>
					<? if ($module->_isScheduled("repair.{$device['DEVID']}")) { ?>
						<b title="<?= tpl_text('Repairs are in progress ...') ?>">...</b>
					<? } else { ?>
						<a href="<?= tpl_link_call('tracker','repair',$device['DEVID']) 
							?>"><img src="<?= tpl_design_uri('image/icon/16x16/repair.gif') ?>" alt="<?= tpl_text('Repair') 
							?>" title="<?= tpl_text('Repair this device (might take a while)') ?>" width="16" height="16" /></a>
					<? } ?>
				</td>
			</tr>
			<? } ?>
			<tr class="<?= tpl_cycle() ?>">
				<td><input type="text" name="device[<?= $namespace['NSID'] ?>][URN]" size="40" /></td>
				<td>&nbsp;</td>
				<td>&nbsp;</td>
				<td>&nbsp;</td>
				<td style="border-left:1px solid;">&nbsp;</td>
				<td>&nbsp;</td>
				<td class="Numeric"><input type="text" name="device[<?= $namespace['NSID'] ?>][SPACE]" size="4" /></td>
				<td style="border-left:1px solid;">&nbsp;</td>
				<td>&nbsp;</td>
				<td>&nbsp;</td>
				<td>&nbsp;</td>
				<td>&nbsp;</td>
			</tr>
		<? } ?>
	</table>
	<p class="Help"><?= tpl_text('Each namespace has its own storage pool with a space allotment per physical device. Additional physical devices may be added with an URN and a space allotment, for example <em>\'xmlrpc;http://node01.example.com/synd/xmlrpc/device/\'</em>') ?></p>
	<p style="text-align:right;">
		<input type="submit" value="<?= tpl_text('Save') ?>" />
	</p>
</form>

<h3><?= tpl_text('Maintenance tasks') ?></h3>
<ul class="Actions">
<? if ($replScheduled) { ?>
	<li><?= tpl_text('Replication routine is running ...') ?></li>
<? } else { ?>
	<li><a href="<?= tpl_link_call('tracker','replicate') ?>"><?= tpl_text('Start replication routine') ?></a></li>
<? } ?>
</ul>
