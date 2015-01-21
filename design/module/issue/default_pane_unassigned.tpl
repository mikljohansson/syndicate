<?
global $synd_user; 
require_once 'design/gui/PagedListing.class.inc';

$projects = $module->getProjectTree(new MethodDecider('hasPermission', array($synd_user, 'manage')), $synd_user);
$projects->rewind();

if ($projects->valid()) {
	$query = synd_node_issue::getEntityQuery(SyndNodeLib::getDefaultStorage('issue'));
	$i = $query->join('synd_issue');

	$query->in("$i.parent_node_id", new MemberIterator($projects, 'nodeId'));
	$query->where("$i.assigned_node_id IS NULL");
	$query->where("$i.info_status >= 0 AND $i.info_status < ".synd_node_issue::CLOSED);
	
	if (!count($order = tpl_sort_order('issue')))
		$order = array('TS_RESOLVE_BY');

	$issues = $query->getEntities();
	$offset = (int)$request['offset'];
	$limit = 50;
}

?>
<? if (($count = count($issues))) { ?>
	<div class="Result">
		<table style="width:100%;">
			<tr>
				<td>
					<?= tpl_text('Displaying %d-%d of %d unassigned issues', 
						$offset+1, $offset+min($count-$offset,$limit), $count) ?><br />
					<? $this->display(tpl_design_path('gui/pager.tpl'),array('offset'=>$offset,'limit'=>$limit,'count'=>$count)) ?>
				</td>
				<td style="text-align:right; vertical-align:bottom;">
					<? include tpl_design_path('module/issue/issue_status_legend.tpl') ?>
				</td>
			</tr>
		</table>
	</div>
	<? $this->display('model/node/issue/table_assigned.tpl', array('list'=>$issues->getIterator($offset,$limit,$order))) ?>
	<? if ($count > $limit) { ?>
	<div class="Result">
		<? $this->display(tpl_design_path('gui/pager.tpl'),array('offset'=>$offset,'limit'=>$limit,'count'=>$count)) ?>
	</div>
	<? } ?>
<? } else { ?>
	<div class="Result">
		<?= tpl_text('No unassigned issues found.') ?>
	</div>
<? } ?>