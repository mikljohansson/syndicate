<? tpl_sort_list($_list, array('INFO_HEAD')); ?>
<table class="Items" width="100%" cellpadding="2">
	<thead>
		<tr>
			<th width="10">&nbsp;</th>
			<th><?= tpl_text('Name of VLAN') ?></th>
			<th><?= tpl_text('Description') ?></th>
			<th width="10">
				<? if (isset($collection)) { ?>
				<?= tpl_form_checkbox('collections[]',false,$collection->id()) ?>
				<? } else print '&nbsp;'; ?>
			</th>
		</tr>
	</thead>
	<tbody>