<? tpl_sort_list($_list, 'costcenter') ?>
<table width="100%" cellpadding="2">
<thead>
	<tr>
		<th style="width:10px;">&nbsp;</th>
		<th style="width:1%;"><a href="<?= tpl_sort_uri('costcenter','INFO_NUMBER') ?>"><?= tpl_text('Number') ?></a></th>
		<th style="width:1%;"><a href="<?= tpl_sort_uri('costcenter','INFO_PROJECT_CODE') ?>"><?= tpl_text('Project') ?></a></th>
		<th><a href="<?= tpl_sort_uri('costcenter','INFO_HEAD') ?>"><?= tpl_text('Name') ?></a></th>
		<th><?= tpl_text('Description') ?></th>
	</tr>
</thead>
<tbody>