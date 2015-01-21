<h1><?= $module->getDescription() ?></h1>

<? if (!empty($projects)) { ?>
<table>
	<? foreach (array_keys($projects) as $key) { ?>
	<tr>
		<td style="white-space:nowrap; padding-right:15px;">
			<a href="<?= tpl_link('plan','view',$projects[$key]->nodeId) ?>"><?= tpl_def($projects[$key]->toString()) ?></a>
		</td>
		<td><?= tpl_def($projects[$key]->getDescription()) ?></td>
	</tr>
	<? } ?>
</table>
<? } ?>