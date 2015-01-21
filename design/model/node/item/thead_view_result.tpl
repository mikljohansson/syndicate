<? tpl_sort_list($_list, 'item') ?>
<table width="100%" cellpadding="2">
<thead>
	<tr>
		<th width="10">&nbsp;</th>
		<th width="1%"><a href="<?= tpl_sort_uri('item','INFO_SERIAL_INTERNAL') ?>"><?= tpl_text('S/N') ?></a></th>
		<th width="1%"><a href="<?= tpl_sort_uri('item','INFO_SERIAL_MAKER') ?>"><?= tpl_text('Mkr&nbsp;S/N') ?></a></th>
		<th width="1%"><a href="<?= tpl_sort_uri('item','INFO_MAKE') ?>"><?= tpl_text('Make') ?></a></th>
		<th><?= tpl_text('Client') ?></th>
		<th><a href="<?= tpl_sort_uri('item','PARENT_NODE_ID') ?>"><?= tpl_text('Folder') ?></a></th>
		<th width="10">
			<? if (isset($collection)) { ?>
			<?= tpl_form_checkbox('collections[]',false,$collection->id()) ?>
			<? } else print '&nbsp;'; ?>
		</th>
	</tr>
</thead>
<tbody>