<div class="Article">
	<? if ($node->isNew()) { ?>
	<input type="hidden" name="stack[]" value="<?= tpl_link_jump($node->getHandler(),'edit',$node->nodeId) ?>" />
	<? } ?>
	
	<div class="RequiredField">
		<h3><?= tpl_text('Category name') ?></h3>
		<input type="text" name="data[NAME]" value="<?= $data['NAME'] ?>" size="30" />
	</div>

	<div class="OptionalField">
		<?= tpl_form_checkbox('data[FLAG_LEASE_ONLY]',$data['FLAG_LEASE_ONLY']) ?>
			<?= tpl_text('Items in this category is lease only.') ?>
	</div>

	<? if (count($fields = $node->getFields())) { ?>
	<div class="OptionalField">
		<h3><?= tpl_text('Category specific fields') ?></h3>
		<table style="width:400px;">
			<tr>
				<th><?= tpl_text('Name') ?></th>
				<th><?= tpl_text('Datatype') ?></th>
				<th width="1%">&nbsp;</th>
			</tr>
			<? foreach (array_keys($fields) as $key) { ?>
			<tr class="<?= tpl_cycle(array('odd','even')) ?>">
				<td><?= $fields[$key]->toString() ?></td>
				<td><?= $fields[$key]->getDatatypeName() ?>
				<td><a href="<?= tpl_link_call('inventory','invoke',$node->nodeId,'delField',$key)
					?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= tpl_text('Remove') ?>" /></a></td>
			</tr>
			<? } ?>
		</table>
		<br />
	</div>
	<? } ?>

	<? if (!$node->isNew()) { ?>
	<div class="OptionalField">
		<h3><?= tpl_text('Add field') ?></h3>
		<input type="text" name="data[field][INFO_HEAD]" />
		<select name="data[field][INFO_DATATYPE]">
			<?= tpl_form_options(SyndLib::invoke($node->getDatatypes(),'getDatatypeName'),$field['INFO_DATATYPE']) ?>
		</select>
		<input type="submit" value="<?= tpl_text('Add') ?>" />
	</div>
	<? } ?>

	<p>
		<span title="<?= tpl_text('Accesskey: %s','S') ?>">
			<input accesskey="s" type="submit" name="post" value="<?= $node->isNew()?tpl_text('Add item'):tpl_text('Save') ?>" />
		</span>
		<span title="<?= tpl_text('Accesskey: %s','A') ?>">
			<input accesskey="a" type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
		</span>
		<? if (!$node->isNew()) { 
			$parent = $node->getParent(); ?>
		<input class="button" type="button" value="<?= tpl_text('Delete') ?>" onclick="window.location='<?= 
			tpl_view_call($node->getHandler(),'delete',$node->nodeId) ?>';" />
		<? } ?>
	</p>
</div>