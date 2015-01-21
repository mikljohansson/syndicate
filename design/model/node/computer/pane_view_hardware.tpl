<table class="Vertical">
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Processor') ?></th>
		<td>
			<?= tpl_def($node->data['INFO_CPU_DESC']) ?>
			<? if ($node->data['INFO_CPU_COUNT'] > 1) { ?>
				x <?= $node->data['INFO_CPU_COUNT'] ?>
			<? } ?>
			<? if ($node->data['INFO_CPU_CLOCK']) { ?>
				<?= $node->data['INFO_CPU_CLOCK'] ?>MHz
			<? } ?>
			<? if ($node->data['INFO_CPU_ID']) { ?>
			<?= tpl_text('(CPUID %s)', $node->data['INFO_CPU_ID']) ?>
			<? } ?>
		</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('RAM') ?></th>
		<td><?= tpl_def($node->data['INFO_RAM']) ?>Mb</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Motherboard') ?></th>
		<td>
			<?= tpl_def($node->data['INFO_MB_MAKE']) ?>
			<? if ($node->data['INFO_CPU_ID']) { ?>
			<?= tpl_text('(Serial %s)', $node->data['INFO_MB_SERIAL']) ?>
			<? } ?>
		</td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('BIOS') ?></th>
		<td><?= tpl_def($node->data['INFO_MB_BIOS']) ?></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Keyboard') ?></th>
		<td><?= tpl_def($node->data['INFO_KEYBOARD']) ?></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Date') ?></th>
		<td><?= ucwords(tpl_strftime('%A, %d %B %Y %H:%M:%S', $node->data['TS_UPDATE'])) ?></td>
	</tr>
	<? if (!empty($node->data['INFO_REMOTE_URI'])) { ?>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Remote uri') ?></th>
		<td><?= $node->data['INFO_REMOTE_URI'] ?>, <?= strtoupper($node->data['INFO_REMOTE_METHOD']) ?></td>
	</tr>
	<? } ?>
</table>
<br />

<table class="Vertical">
	<caption><?= tpl_text('Disk drives') ?></caption>
	<tr>
		<th><?= tpl_text('Description') ?></th>
		<th><?= tpl_text('Size') ?></th>
		<th><?= tpl_text('Cache') ?></th>
	</tr>
	<? foreach ($node->getDiskDrives() as $disk) { ?>
	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<td><?= tpl_def($disk['INFO_DESC']) ?></td>
		<td>
			<? if ($disk['INFO_SIZE']) { ?>
				<? if ($disk['INFO_SIZE'] > 4000) { ?>
					<?= round($disk['INFO_SIZE']/1000,1) ?>Gb
				<? } else { ?>
					<?= tpl_def($disk['INFO_SIZE']) ?>Mb
				<? } ?>
			<? } else print '&nbsp;' ?>
		</td>
		<td>
			<? if ($disk['INFO_CACHE']) { ?>
				<?= $disk['INFO_CACHE'] ?>Kb
			<? } else print '&nbsp;' ?>
		</td>
	</tr>
	<? } ?>
</table>
<br />

<table class="Vertical">
	<caption><?= tpl_text('CDROM/DVD drives') ?></caption>
	<tr>
		<th><?= tpl_text('Description') ?></th>
		<th><?= tpl_text('Mountpoint') ?></th>
	</tr>
	<? foreach ($node->getROMDrives() as $drive) { ?>
	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<td><?= tpl_def($drive['INFO_DESC']) ?></td>
		<td><?= tpl_def($drive['INFO_MOUNTPOINT']) ?></td>
	</tr>
	<? } ?>
</table>
<br />

<table class="Vertical">
	<caption><?= tpl_text('Graphic cards') ?></caption>
	<tr>
		<th><?= tpl_text('Description') ?></th>
		<th><?= tpl_text('RAM') ?></th>
		<th><?= tpl_text('Size') ?></th>
	</tr>
	<? foreach ($node->getVideoControllers() as $card) { ?>
	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<td><?= tpl_def($card['INFO_DESC']) ?></td>
		<td>
			<? if (!empty($card['INFO_RAM'])) { ?>
				<?= $card['INFO_RAM'] ?>Mb
			<? } else print '&nbsp;' ?>
		</td>
		<td>
			<? if (!empty($card['INFO_HRES'])) { ?>
				<?= $card['INFO_HRES'] ?>x<?= $card['INFO_VRES'] ?>
			<? } else print '&nbsp;' ?>
			<? if (!empty($card['INFO_FREQ'])) { ?>
			@ <?= $card['INFO_FREQ'] ?>Hz
			<? } ?>
			<? if (!empty($card['INFO_BITS'])) { ?>
			<?= $card['INFO_BITS'] ?>bit
			<? } ?>
		</td>
	</tr>
	<? } ?>
</table>
<br />

<table class="Vertical">
	<caption><?= tpl_text('Monitors') ?></caption>
	<tr>
		<th><?= tpl_text('Description') ?></th>
		<th><?= tpl_text('Vendor Id') ?></th>
		<th><?= tpl_text('Size') ?></th>
	</tr>
	<? foreach ($node->getMonitors() as $monitor) { ?>
	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<td><?= tpl_def($monitor['INFO_DESC']) ?></td>
		<td><?= tpl_def($monitor['INFO_VENDOR_ID']) ?></td>
		<td><?= tpl_def(round($monitor['INFO_SIZE'],1)) ?></td>
	</tr>
	<? } ?>
</table>
<br />

<table class="Vertical">
	<caption><?= tpl_text('Sound cards') ?></caption>
	<tr>
		<th><?= tpl_text('Description') ?></th>
	</tr>
	<? foreach ($node->getSoundDevices() as $card) { ?>
	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<td><?= tpl_def($card['INFO_DESC']) ?></td>
	</tr>
	<? } ?>
</table>
