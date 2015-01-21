<table class="License">
	<thead>
		<tr>
			<th><h2><?= $node->data['INFO_MAKE'] ?> <?= $node->toString() ?></h2></th>
		</tr>
		<tr>
			<th>
				<?= $node->isSiteLicense() ? tpl_text('Site (unlimited) license') : 
					tpl_text('%d licenses (%d in use)', $node->getLicenses(), $node->getUsedLicenses()) ?>
			</th>
	</thead>
</table>
<? if ($node->isPermitted('write')) { ?>
<ul class="Actions">
	<li><a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>"><?= tpl_text('Edit this license') ?></a></li>
</ul>
<? } ?>

<h2><?= tpl_text('Folders') ?></h2>
<div class="Info"><?= tpl_text('The folders for which this license should be used') ?></div>
<? if (count($folders = $node->getFolders())) { ?>
	<? tpl_sort_list($folders,'folder') ?>
	<table width="100%">
	<thead>
		<tr>
			<th><a href="<?= tpl_sort_uri('folder','INFO_HEAD') ?>"><?= tpl_text('Name') ?></a></th>
			<th><a href="<?= tpl_sort_uri('folder','INFO_DESC') ?>"><?= tpl_text('Description') ?></a></th>
			<? if ($node->isPermitted('write')) { ?>
			<th width="1%">&nbsp;</th>
			<? } ?>
		</tr>
	</thead>
	<tbody>	
		<? foreach (array_keys($folders) as $key) { ?>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td><?= tpl_value($folders[$key]->toString()) ?></td>
			<td><?= tpl_value($folders[$key]->getDescription()) ?></td>
			<? if ($node->isPermitted('write')) { ?>
			<td><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'removeFolder',$folders[$key]->nodeId) ?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= tpl_text('Remove') ?>" /></a></td>
			<? } ?>
		</tr>
		<? } ?>
	</tbody>
	</table>
<? } ?>

<br />
<form action="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'addFolder') ?>" method="post">
	<select name="folder">
		<? $this->display(tpl_design_path('module/inventory/folder_options.tpl'),
			array('selected' => SyndNodeLib::getInstance($data['PARENT_NODE_ID']))); ?>
	</select>
	<input type="submit" value="<?= tpl_text('Add folder &gt;&gt;') ?>" />
</form>

<br />
<h2><?= tpl_text('Software products') ?></h2>
<div class="Info"><?= tpl_text('The software product strings that should be matched against, wildcards ? and * are supported.') ?></div>
<? if (count($filters = $node->getSoftwareFilters())) { ?>
	<table width="100%">
	<tbody>	
		<? 
		sort($filters);
		foreach ($filters as $filter) { ?>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td><?= tpl_value($filter) ?></td>
			<? if ($node->isPermitted('write')) { ?>
			<td width="1%"><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'delSoftwareFilter',$filter) ?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" alt="<?= tpl_text('Remove') ?>" /></a></td>
			<? } ?>
		</tr>
		<? } ?>
	</tbody>
	</table>
<? } ?>

<br />
<form action="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'addSoftwareFilter') ?>" method="post">
	<input type="text" name="filter" size="50" maxlength="512" />
	<input type="submit" value="<?= tpl_text('Add filter &gt;&gt;') ?>" />
</form>

<h3><?= tpl_text('Software matching the current filters') ?></h3>
<? if (count($filters)) {
	$sql = "
		SELECT s.info_product, COUNT(*) 
		FROM synd_instance i, synd_inv_software s
		WHERE 
			i.node_id = s.os_node_id AND
			i.ts_update >= ".strtotime('-90 days')." AND
			(LOWER(s.info_product) LIKE ".strtr(implode(' OR LOWER(s.info_product) LIKE ', 
				$node->_db->quote(array_map('strtolower',$filters))), '?*', '_%').")
		GROUP BY s.info_product";
	$software = $node->_db->getAssoc($sql); 
	
	?>
	<table width="100%">
		<thead>
			<th><?= tpl_text('Product') ?></th>
			<th style="width:1%; white-space:nowrap; text-align:right;"><?= tpl_text('Total installations') ?></th>
		</thead>
		<tbody>
			<? foreach ($software as $product => $count) { ?>
			<tr class="<?= tpl_cycle(array('odd','even')) ?>">
				<td><?= tpl_value($product) ?></td>
				<td style="text-align:right;"><?= tpl_value($count) ?></td>
			</tr>
			<? } ?>
		</tbody>
		<tfoot>
			<tr>
				<th><?= tpl_text('Sum') ?></th>
				<th style="text-align:right;"><?= tpl_value(array_sum($software)) ?></th>
			</tr>
		</tfoot>
	</table>
<? } else { ?>
<div class="Info"><?= tpl_text('No software matching the current filters') ?></div>
<? } ?>


