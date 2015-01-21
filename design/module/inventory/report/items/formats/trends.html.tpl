<? require tpl_design_path('module/inventory/report/items/formats/trends.inc'); ?>

<table>
	<thead>
		<tr>
			<th>&nbsp;</th>
			<th class="Numeric"><?= tpl_text('Optimistic') ?><sup>1</sup></th>
			<th class="Numeric"><?= tpl_text('Pessimistic') ?><sup>2</sup></th>
		</tr>
	</thead>
	<tbody>
		<? foreach (array_keys($groups) as $key) { ?>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td><?= is_object($groups[$key]) ? $groups[$key]->toString() : $groups[$key] ?></td>
			<td class="Numeric"><?= tpl_value($optimistic[$key]) ?></td>
			<td class="Numeric"><?= tpl_value($pessimistic[$key]) ?></td>
		</tr>
		<? } ?>
	</tbody>
	<? if (empty($hideSummary)) { ?>
	<tfoot>
		<tr>
			<th><?= tpl_text('Sum') ?></th>
			<th class="Numeric"><?= tpl_value(array_sum($optimistic)) ?></th>
			<th class="Numeric"><?= tpl_value(array_sum($pessimistic)) ?></th>
		</tr>
	</tfoot>
	<? } ?>
</table>

<p><sup>1)</sup> <?= tpl_text('Optimistic values assume that all currently expired leases will be settled') ?></p>
<p><sup>2)</sup> <?= tpl_text('Pessimistic values assume that none of the currently expired and unreturned leases will be settled') ?></p>
