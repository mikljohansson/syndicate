<table>
<thead>
	<tr>
		<td>&nbsp;</td>
		<td>
			<a href="<?= tpl_sort_uri('attempt','PARENT_NODE_ID') ?>">
			<b><?= tpl_text('Chapter') ?></b></a>
		</td>
		<td>
			<a href="<?= tpl_sort_uri('attempt','TS_CREATE') ?>">
			<b><?= tpl_text('Date') ?></b></a>
		</td>
		<td><b><?= tpl_text('Correct') ?></b></td>
		<td>&nbsp;</td>
	</tr>
</thead>
<tbody>
	<? $this->iterate($list, 'trow_view.tpl', $_data) ?>
</tbody>
</table>
