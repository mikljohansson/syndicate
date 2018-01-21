<table class="Vertical">
	<tbody>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<th>&nbsp;</th>
			<td>
				<ul class="Actions">
					<li><a href="<?= tpl_link_call('log','delete',$request[1]) ?>"><?= tpl_text('Delete this exception') ?></a></li>
				</ul>
			</td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Timestamp') ?></th>
			<td><?= tpl_strftime('%Y-%m-%d %H:%M:%S', $error['TIMESTAMP']) ?></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Uri') ?></th>
			<td><a href="<?= $error['REQUEST_URI'] ?>"><?= $error['REQUEST_URI'] ?></a></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('File') ?></th>
			<td><?= $error['FILENAME'] ?>:<?= $error['LINE'] ?></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('HTTP status') ?></th>
			<td><?= $error['STATUS'] ?></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Code') ?></th>
			<td><?= $module->_formatExceptionType($error['CODE']) ?></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Message') ?></th>
			<td><?= tpl_def($error['MESSAGE']) ?></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Description') ?></th>
			<td><?= tpl_def($error['DESCRIPTION']) ?></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('User') ?></th>
			<td><? $this->render(SyndLib::getInstance($error['CLIENT_NODE_ID']),'head_view.tpl') ?></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Stacktrace') ?></th>
			<td><pre><?= $error['STACKTRACE'] ?></pre></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th><?= tpl_text('Context') ?></th>
			<td><pre><?= $error['CONTEXT'] ?></pre></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th>$_REQUEST</th>
			<td><pre><?= $module->_context($error['REQUEST_DATA']) ?></pre></td>
		</tr>
		<tr class="<?= tpl_cycle() ?>">
			<th>$_SERVER</th>
			<td><pre><?= $module->_context($error['REQUEST_ENV']) ?></pre></td>
		</tr>
	</tbody>
</table>
