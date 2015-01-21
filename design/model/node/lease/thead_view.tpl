<? tpl_sort_list($_list, 'lease') ?>
<table width="100%" cellpadding="2">
<thead>
	<tr>
		<th width="10">&nbsp;</th>
		<th><a href="<?= tpl_sort_uri('lease','CLIENT_NODE_ID') ?>"><?= tpl_text('Client') ?></a></th>
		<th><a href="<?= tpl_sort_uri('lease','TS_CREATE') ?>"><?= tpl_text('Created') ?></a></th>
		<th><a href="<?= tpl_sort_uri('lease','TS_EXPIRE') ?>"><?= tpl_text('Expires') ?></a></th>
		<th><a href="<?= tpl_sort_uri('lease','TS_EXPIRE') ?>"><?= tpl_text('Terminated') ?></a></th>
		<th width="10">
			<? if (isset($collection)) { ?>
			<?= tpl_form_checkbox('collections[]',false,$collection->id()) ?>
			<? } else print '&nbsp;'; ?>
		</th>
	</tr>
</thead>
<tbody>