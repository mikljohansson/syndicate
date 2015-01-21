<table class="Items" width="100%" cellpadding="2">
	<thead>
		<tr>
			<th width="10">&nbsp;</th>
			<th width="1%"><a href="<?= tpl_sort_uri('item','INFO_MAKE') ?>"><?= tpl_text('Model') ?></a></th>
			<th width="1%"><a href="<?= tpl_sort_uri('item','INFO_SERIAL_MAKER') ?>"><?= tpl_text('S/N') ?></a></th>
			<th width="1%"><a href="<?= tpl_sort_uri('item','INFO_SERIAL_INTERNAL') ?>"><?= tpl_text('Internal&nbsp;S/N') ?></a></th>
			<th><?= tpl_text('Customer') ?></th>
			<? if (empty($hideCheckbox)) { ?>
			<th width="10">
				<? if (isset($collection)) { ?>
				<?= tpl_form_checkbox('collections[]',false,$collection->id()) ?>
				<? } else print '&nbsp;'; ?>
			</th>
			<? } ?>
		</tr>
	</thead>
	<tbody>