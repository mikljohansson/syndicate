	<h2><?= tpl_text('Options for this folder') ?></h2>
	<table>
		<? if (count($values = $node->getOptionalValues())) { ?>
		<tbody>
			<? foreach ($values as $key => $value) { ?>
			<tr class="<?= tpl_cycle(array('odd','even')) ?>">
				<td><b><?= tpl_def($key) ?></b></td>
				<td><?= tpl_def($value) ?></td>
				<td><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'delOptionalValue',$key) 
					?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= tpl_text('Remove') ?>" /></a></td>
			</tr>
			<? } ?>
		</tbody>
		<? } ?>
		<tfoot>
			<? if (count($definitions = $node->getOptionalDefinitions())) { ?>
			<tr class="<?= tpl_cycle(array('odd','even')) ?>">
				<td>
					<select name="data[value][0]">
						<? foreach (array_keys($definitions) as $key) { ?>
						<option value="<?= $definitions[$key]->toString() ?>"><?= $definitions[$key]->toString() ?></option>
						<? } ?>
					</select>
				</td>
				<td><input type="text" name="data[value][1]" size="15" /></td>
				<td><input type="submit" value="<?= tpl_text('Add') ?>" /></td>
			</tr>
			<? } else { ?>
			<tr>
				<td><em><?= tpl_text('Select a category for this folder before specifying options') ?></em></td>
			</tr>
			<? } ?>
		</tfoot>
	</table>


	<? $parent = $node->getParent(); if (!$parent->isNull() && count($values = $parent->getOptionalValues())) { ?>
	<h2><?= tpl_text('Inherited options') ?></h2>
	<table>
	<? foreach ($values as $key => $value) { ?>
		<tr>
			<td><?= tpl_def($key) ?></td>
			<td><?= tpl_def($value) ?></td>
		</tr>
	<? } ?>
	</table>
	<? } ?>

	<? if (!empty($definitions)) { ?>
	<p>
		<span title="<?= tpl_text('Accesskey: %s','S') ?>">
			<input accesskey="s" type="submit" name="post" value="<?= tpl_text('Save') ?>" />
		</span>
		<input type="button" value="<?= tpl_text('Abort') ?>" onclick="history.go(-1)" />
		<? if (!$node->isNew()) { ?>
		<input class="button" type="button" value="<?= tpl_text('Delete') ?>" onclick="window.location='<?= 
			tpl_view($node->getHandler(),'delete',$node->nodeId) ?>';" />
		<? } ?>
	</p>
	<? } ?>
