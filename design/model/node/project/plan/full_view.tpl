<? $rlisting = $node->getResourceAllocation(); ?>
<input type="hidden" name="collections[]" value="<?= $node->id() ?>" />
<div class="Article">
	<div class="Header">
		<h1><?= $node->getTitle() ?></h1>
	</div>

	<? if ($rlisting->getCount()) { 
		$result = $rlisting->getResult(); ?>
		<div class="Result">
			<?= tpl_text('Results %d-%d of %d resources allocated to this project', 
				$rlisting->getOffset()+1, $rlisting->getOffset()+count($resources), $rlisting->getCount()) ?>
			<? $this->display(tpl_design_path('gui/pager.tpl'),$rlisting->getParameters()) ?>
		</div>
		<table class="Resources">
			<thead>
				<tr>
					<th width="10">&nbsp;</th>
					<th><a href="<?= tpl_sort_uri('resource','INFO_HEAD') ?>"><?= tpl_text('Name') ?></a></th>
				</tr>
			</thead>
			<tbody>		
				<? while (false != ($row = $result->fetchRow())) { 
					if (null !== ($resource = $node->_storage->getInstance($row['NODE_ID']))) { ?>
				<tr class="<?= tpl_cycle(array('odd','even')) ?>">
					<td><? if ($resource->isPermitted('write')) { ?><a href="<?= tpl_link_call('plan','edit',$resource->nodeId) ?>"><img src="<?= tpl_design_uri('image/icon/record.gif') ?>" alt="" /></a><? } else print '&nbsp;'; ?></td>
					<td><a href="<?= tpl_link('plan','view',$resource->nodeId) ?>"><?= tpl_def($resource->toString()) ?></a></td>
				</tr>
				<? }} ?>
			</tbody>
		</table>		
		<? } ?>

	<? if (count($projects = $node->getProjects())) { ?>
	<?= tpl_gui_table('project',$projects,'view_plan.tpl') ?>
	<? } ?>
</div>