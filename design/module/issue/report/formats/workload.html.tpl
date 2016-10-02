<?
require_once 'core/lib/SyndDate.class.inc';
include tpl_design_path('module/issue/context.tpl');

$limit = 50;
$offset = isset($_REQUEST['offset']) ? $_REQUEST['offset'] : 0;
$issues = $report->getContents($offset, $limit);
$count = $report->getCount();

require tpl_design_path('module/issue/report/formats/workload.inc');

?>
<div class="Result">
	<?= tpl_text("Results %d-%d of %d", $offset+1, $offset+count($issues), $count) ?><br />
	<? $this->display(tpl_design_path('gui/pager.tpl'),
		array('limit'=>$limit,'offset'=>$offset,'count'=>$count)) ?>
</div>

<table class="Issues">
	<? foreach ($month as $monthDate => $monthIssues) { ?>
	<thead>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<th colspan="3"><?= $monthDate ?></th>
			<th><a href="<?= tpl_sort_uri('issue','TS_RESOLVE_BY') ?>"><?= tpl_text('Due') ?></a></th>
			<th><a href="<?= tpl_sort_uri('issue','INFO_PRIO') ?>"><?= tpl_text('Prio') ?></a></th>
			<th class="Numeric"><a href="<?= tpl_sort_uri('issue','INFO_ESTIMATE') ?>"><?= tpl_text('Estimate') ?></a></th>
			<th class="Numeric"><?= tpl_text('Logged') ?></th>
			<th>&nbsp;</th>
			<th><input type="checkbox" onclick="Issues.select(this);" /></th>
		</tr>
	</thead>
	<tbody>
		<? foreach ($week[$monthDate] as $weekDate => $issues) { ?>
		<tr class="Selected">
			<td colspan="5"><em><?= $weekDate ?></em></td>
			<td class="Numeric"><em><?= tpl_duration(array_sum(SyndLib::invoke($issues,'getEstimate')),null,'h','m') ?></em></td>
			<td class="Numeric"><em><?= tpl_duration(array_sum(SyndLib::invoke($issues,'getDuration')),null,'h','m') ?></em></td>
			<td>&nbsp;</td>
			<td>&nbsp;</td>
		</tr>
			<? foreach (array_keys($issues) as $key) { ?>
			<tr class="<?= tpl_cycle(array('odd','even')) ?> <?= $issues[$key]->getStatusName() ?><?= $issues[$key]->isOverdue()?' Overdue':'' ?> <?= $issues[$key]->getPriorityName() 
				?>" oncontextmenu="return issue_context_menu(this,event,'<?= tpl_view('rpc','json',$issues[$key]->id()) ?>',true);" id="<?= $issues[$key]->id() ?>">
				<td class="nowrap"><? 
					if ($issues[$key]->isPermitted('write')) { 
						?><a title="<?= tpl_text('Edit this issue') ?>" href="<?= tpl_link_call($issues[$key]->getHandler(),'edit',$issues[$key]->nodeId) ?>"><img src="<?= tpl_design_uri('image/icon/record.gif') ?>" /></a><? 
					} else print '&nbsp;' ?>
				</td>
				<td class="nowrap">
					<? $assigned = $issues[$key]->getAssigned(); ?>
					<a href="<?= tpl_link('issue','resport',array('assigned'=>$assigned->nodeId)) ?>"><?= tpl_chop($assigned->toString(),25) ?></a>
				</td>
				<td><a href="<?= tpl_link($issues[$key]->getHandler(),$issues[$key]->objectId()) ?>" title="<?= 
					tpl_attribute($issues[$key]->getExcerpt()) ?>"><?= $issues[$key]->getTitleCategories() ?> <?= 
					synd_htmlspecialchars(tpl_chop($issues[$key]->getTitle(),65)) ?></a></td>
				<td class="Due" title="<?= ucwords(tpl_strftime('%A, %d %B %Y', $issues[$key]->getResolveBy())) ?>"><?= 
					ucwords(tpl_strftime('%a, %d %b', $issues[$key]->getResolveBy())) ?></td>
				<td>
					<? switch ($issues[$key]->getPriority()) { 
						case 0: print tpl_text('Low'); break;
						case 1: print tpl_text('Norm'); break;
						case 2: print tpl_text('High'); break;
					} ?>
				</td>
				<td class="Numeric"><?= tpl_def(tpl_duration($issues[$key]->getEstimate(),null,'h','m')) ?></td>
				<td class="Numeric"><?= tpl_def(tpl_duration($issues[$key]->getDuration(),null,'h','m')) ?></td>
				<td class="Status"><div><?= tpl_default($issues[$key]->getOpenCount(),'<img src="'.tpl_design_uri('image/pixel.gif').'" alt="" />') ?></div></td>
				<td class="OLE" onmouseover="this.parentNode.setAttribute('_checked','true');" onmouseout="this.parentNode.setAttribute('_checked',this.firstChild.checked);"><input type="checkbox" name="selection[]" value="<?= $issues[$key]->id() ?>" /></td>
			</tr>
			<? } ?>
		<? } ?>		
	</tbody>
	<tfoot>
		<tr>
			<th colspan="5">&nbsp;</th>
			<th class="Numeric"><?= tpl_duration(array_sum(SyndLib::invoke($monthIssues,'getEstimate')),null,'h','m') ?></th>
			<th class="Numeric"><?= tpl_duration(array_sum(SyndLib::invoke($monthIssues,'getDuration')),null,'h','m') ?></th>
			<th>&nbsp;</th>
			<th>&nbsp;</th>
		</tr>
		<tr><td colspan="6">&nbsp;</td></tr>
	</tfoot>
	<? } ?>
</table>
