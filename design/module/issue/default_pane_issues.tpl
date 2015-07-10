<? 
global $synd_user; 
require_once 'design/gui/PagedListing.class.inc';

$storage = SyndNodeLib::getDefaultStorage('issue');
$database = $storage->getDatabase();

$query = synd_node_issue::getEntityQuery($storage);
$i = $query->join('synd_issue');

// Issues assigned to user
$assigned = clone $query;
$p = $assigned->join('synd_project', 'p', false);

$assigned->where("$i.parent_node_id = $p.node_id");
$assigned->where("$i.assigned_node_id", $synd_user->nodeId);
$assigned->where("$i.info_status >= 0 AND $i.info_status < ".synd_node_issue::CLOSED);
$assigned->where("($p.flag_hide_issues = 0 OR $i.ts_resolve_by < ".time().")");

if (empty($request['lowprio'])) {
	// Filter out low priority issues
	$assigned->where("($i.info_prio > 0 OR $i.ts_resolve_by < ".time()." OR $i.info_status = ".synd_node_issue::ACTIVE.")");

	// Filter out subissues
	$assigned->where("
		($i.issue_node_id IS NULL OR NOT EXISTS (
			SELECT 1 FROM synd_issue i0 
			WHERE 
				i0.node_id = $i.issue_node_id AND 
				i0.assigned_node_id = $i.assigned_node_id))");
}

$sql = "
	SELECT COUNT(*) FROM synd_issue i, synd_project p
	WHERE 
		i.parent_node_id = p.node_id AND
		i.assigned_node_id = ".$database->quote($synd_user->nodeId)." AND
		i.info_status >= 0 AND i.info_status < ".synd_node_issue::CLOSED." AND
		(((p.flag_hide_issues = 0 OR i.ts_resolve_by < ".time().") AND
		  (i.info_prio = 0 AND i.ts_resolve_by >= ".time()." AND i.info_status < ".synd_node_issue::ACTIVE."))
		 OR
		 (i.issue_node_id IS NOT NULL AND EXISTS (
			SELECT 1 FROM synd_issue i0 
			WHERE 
				i0.node_id = i.issue_node_id AND 
				i0.assigned_node_id = i.assigned_node_id))
		)";
$lowprio = $database->getOne($sql);

// Open issues reported by logged-in user
$open = clone $query;
$open->where("$i.client_node_id", $synd_user->nodeId);
$open->where("($i.assigned_node_id != ".$database->quote($synd_user->nodeId)." OR $i.assigned_node_id IS NULL)");
$open->where("$i.info_status >= 0 AND $i.info_status < ".synd_node_issue::CLOSED);

// Closed issues reported by logged-in user
$closed = clone $query;
$closed->where("$i.client_node_id", $synd_user->nodeId);
$closed->where("($i.assigned_node_id != ".$database->quote($synd_user->nodeId)." OR $i.assigned_node_id IS NULL)");
$closed->where("$i.info_status >= ".synd_node_issue::CLOSED." AND $i.info_status < ".synd_node_issue::MAX_STATUS_VALUE);
$closed->where("$i.ts_resolve > ".strtotime('-1 weeks'));

if (!count($order = tpl_sort_order('issue',"$i.")))
	$order = array("$i.TS_RESOLVE_BY");

?>
<? if (($count = count($assigned)) || $lowprio) { 
	$offset = (int)$request['aoffset'];
	$limit = 50; ?>
	<div class="Result">
		<table style="width:100%;">
			<tr>
				<td>
					<?= tpl_text('Displaying %d-%d of %d assigned issues', $offset+1, $offset+min($count-$offset,$limit), $count) ?>
					<? if ($lowprio) { ?>
						<? if (empty($request['lowprio'])) { ?>
						(<a class="Info" href="<?= tpl_link('issue','issues',array('lowprio'=>1)) ?>"><?= tpl_text('Show %d low-priority and child issues', $lowprio) ?></a>)
						<? } else { ?>
						(<a class="Info" href="<?= tpl_link('issue','issues') ?>"><?= tpl_text('Hide %d low-priority and child issues', $lowprio) ?></a>)
						<? } ?>
					<? } ?>
					<? $this->display(tpl_design_path('gui/pager.tpl'),array('offset'=>$offset,'limit'=>$limit,'count'=>$count,'offset_variable_name'=>'aoffset')) ?>
				</td>
				<td style="text-align:right; vertical-align:bottom;">
					<? include tpl_design_path('module/issue/issue_status_legend.tpl') ?>
				</td>
			</tr>
		</table>
	</div>
	<? $this->display('model/node/issue/table_assigned.tpl',array('list'=>$assigned->getEntities()->getIterator($offset,$limit,$order))) ?>
<? } ?>

<? if (($count = count($open))) {
	$offset = (int)$request['uoffset'];
	$limit = 25; ?>
	<h2><?= tpl_text('Open issues') ?></h2>
	<? $this->display(tpl_design_path('gui/pager.tpl'),array('offset'=>$offset,'limit'=>$limit,'count'=>$count,'offset_variable_name'=>'uoffset')) ?>
	<? $this->display('model/node/issue/table_client.tpl',array('list'=>$open->getEntities()->getIterator($offset,$limit,$order))) ?>
<? } ?>

<? if (($count = count($closed))) { 
	$offset = (int)$request['roffset'];
	$limit = 25; ?>
	<h2><?= tpl_text('Closed issues') ?></h2>
	<? $this->display(tpl_design_path('gui/pager.tpl'),array('offset'=>$offset,'limit'=>$limit,'count'=>$count,'offset_variable_name'=>'roffset')) ?>
	<? $this->display('model/node/issue/table_client.tpl',array('list'=>$closed->getEntities()->getIterator($offset,$limit,$order))) ?>
<? } ?>
