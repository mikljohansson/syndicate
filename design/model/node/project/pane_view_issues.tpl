<?
global $synd_user;
require_once 'design/gui/PagedListing.class.inc';

$query = $node->_db->createQuery();
$i = $query->join('synd_issue', 'i');
$query->column("$i.node_id");
$query->where("$i.parent_node_id = ".$node->_db->quote($node->nodeId));
$query->where("$i.info_status >= 0 AND $i.info_status < ".synd_node_issue::CLOSED);

if ('recent' == $request[0])
	$query->where("$i.ts_create >= ".strtotime('-1 days'));
if (!$node->isPermitted('monitor'))
	$query->where("$i.client_node_id = ".$node->_db->quote($synd_user->nodeId));

if (!count($order = tpl_sort_order('issue',"$i.")))
	$order = array("$i.TS_RESOLVE_BY");

$pager = new PagedListing($node->_storage, $query, $order, 75, null, $request);

?>
<ul class="Actions">
	<li><a href="<?= tpl_link($node->getHandler(),'invoke',$node->nodeId,'newIssue') ?>"><?= 
		tpl_text('New issue for %s',$node->toString()) ?></a></li>
</ul>
<? if (count($issues = $pager->getInstances()) || 'recent' == $request[0]) { ?>
	<div class="Result">
		<table style="width:100%;">
			<tr>
				<td>
					<? if ('recent' == $request[0]) { ?>
						<?= tpl_text('Displaying %d-%d of %d recently arrived issues (<a class="Info" href="%s">Show all issues</a>)', 
							$pager->getOffset()+1, $pager->getOffset()+count($issues), $pager->getCount(), 
							tpl_link($node->getHandler(),'project',$node->getProjectId(),'issues')) ?><br />
					<? } else { ?>
						<?= tpl_text('Displaying %d-%d of %d open issues (<a class="Info" href="%s">Show recent issues</a>)', 
							$pager->getOffset()+1, $pager->getOffset()+count($issues), $pager->getCount(), 
							tpl_link($node->getHandler(),'project',$node->getProjectId(),'recent')) ?><br />
					<? } ?>
					<? $this->display(tpl_design_path('gui/pager.tpl'),$pager->getParameters()) ?>
				</td>
				<td style="text-align:right; vertical-align:bottom;">
					<? include tpl_design_path('module/issue/issue_status_legend.tpl') ?>
				</td>
			</tr>
		</table>
	</div>
	<? $this->display('model/node/issue/table.tpl', array('list'=>$issues)) ?>
	<? if ($pager->getCount() > $pager->getLimit()) { ?>
	<div class="Result">
		<? $this->display(tpl_design_path('gui/pager.tpl'),$pager->getParameters()) ?>
	</div>
	<? } ?>
<? } ?>