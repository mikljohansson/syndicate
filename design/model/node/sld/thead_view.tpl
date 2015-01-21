<? tpl_sort_list($_list, 'costcenter') ?>
<table width="100%" cellpadding="2">
<thead>
	<tr>
		<th style="width:10px;">&nbsp;</th>
		<th><a href="<?= tpl_sort_uri('costcenter','INFO_HEAD') ?>"><?= tpl_text('Name') ?></a></th>
		<th><?= tpl_text('Description') ?></th>
	</tr>
</thead>
<tbody>