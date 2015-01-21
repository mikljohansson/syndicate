<table class="Vertical">
	<caption><?= $node->data['INFO_RELEASE'] ?></caption>
	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<th><?= tpl_text('Version') ?></th>
		<td><?= tpl_def($node->data['INFO_VERSION']) ?></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Machinename') ?></th>
		<td><?= tpl_def($node->data['INFO_MACHINE_NAME']) ?></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Date') ?></th>
		<td><?= ucwords(tpl_strftime('%A, %d %B %Y %H:%M:%S', $node->data['TS_UPDATE'])) ?></td>
	</tr>
	<tr class="<?= tpl_cycle() ?>">
		<th><?= tpl_text('Image') ?></th>
		<td><?= tpl_def($node->data['INFO_LOADED_IMAGE']) ?></td>
	</tr>
</table>

<? if (count($software = $node->getSoftware())) { ?>
<br />
<table class="Vertical">
	<caption><?= tpl_text('Installed software') ?></caption>
	<tr class="<?= tpl_cycle(array('odd','even')) ?>">

		<th><?= tpl_text('Product') ?></th>
		<th style="text-align:right;"><?= tpl_text('Version') ?></th>
	</tr>
	<? foreach ($software as $product) { ?>
	<tr class="<?= tpl_cycle() ?>">
		<td><?= tpl_def($product['INFO_PRODUCT']) ?></td>
		<td style="text-align:right;"><?= tpl_def($product['INFO_VERSION']) ?></td>
	</tr>
	<? } ?>
</table>
<? } ?>