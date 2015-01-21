<ul class="Actions">
	<li><a href="<?= tpl_link_call('log','clear') ?>"><?= tpl_text('Delete all exceptions') ?></a></li>
</ul>
<div class="Result">
	<?= tpl_text('Results %d-%d of %d', $offset+1, $offset+count($errors), $count) ?>
	<? $this->display(tpl_design_path('gui/pager.tpl')); ?>
</div>
<table>
	<thead>
		<tr>
			<th style="width:12em;"><a href="<?= tpl_sort_uri('log_exception','TIMESTAMP') ?>"><?= tpl_text('Timestamp') ?></a></th>
			<th><a href="<?= tpl_sort_uri('log_exception','STATUS') ?>"><?= tpl_text('HTTP') ?></a></th>
			<th><a href="<?= tpl_sort_uri('log_exception','CODE') ?>"><?= tpl_text('Code') ?></a></th>
			<th><?= tpl_text('Message') ?></th>
			<th><a href="<?= tpl_sort_uri('log_exception','REQUEST_URI') ?>"><?= tpl_text('URI') ?></a></th>
			<th style="width:2em;">&nbsp;</th>
		</tr>
	</thead>
	<tbody>
		<? foreach ($errors as $error) { ?>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td><a href="<?= tpl_link('system','log','index',$error['HASH']) ?>"><?= 
				tpl_strftime('%Y-%m-%d %H:%M:%S', $error['TIMESTAMP']) ?></a></td>
			<td><?= tpl_def($error['STATUS']) ?></td>
			<td><?= tpl_def($module->_formatExceptionType($error['CODE'])) ?></td>
			<td><a href="<?= tpl_link('system','log','index',$error['HASH']) ?>"><?= 
				tpl_def(tpl_chop($error['MESSAGE'],55), tpl_text('No message')) ?></a></td>
			<td><?= tpl_chop(tpl_def($error['REQUEST_URI'], $error['FILENAME']),55) ?></td>
			<td><a href="<?= tpl_link_call('log','deleteException',$error['HASH']) ?>"><img src="<?= 
				tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= tpl_text('Delete') ?>" /></a></td>
		</tr>
		<? } ?>
	</tbody>
</table>
