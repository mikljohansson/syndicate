<? tpl_sort_list($_list, 'license') ?>
<table width="100%" cellpadding="2">
<thead>
	<tr>
		<th style="width:10px;">&nbsp;</th>
		<th><a href="<?= tpl_sort_uri('license','INFO_MAKE') ?>"><?= tpl_text('Maker') ?></a></th>
		<th><a href="<?= tpl_sort_uri('license','INFO_PRODUCT') ?>"><?= tpl_text('Product') ?></a></th>
		<th><?= tpl_text('Description') ?></th>
		<th style="text-align:right;"><a href="<?= tpl_sort_uri('license','INFO_LICENSES') ?>"><?= tpl_text('Licenses') ?></a></th>
	</tr>
</thead>
<tbody>