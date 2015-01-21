<?
global $synd_user;
require_once 'design/gui/PagedListing.class.inc';

$query = $node->_db->createQuery();
$i = $query->join('synd_issue', 'i');
$ik = $query->join('synd_issue_keyword', 'ik');

$query->column("$i.node_id");
$query->where("$i.node_id = $ik.issue_node_id");
$query->where("$i.info_status < ".synd_node_issue::CLOSED);
$query->where("$ik.keyword_node_id = ".$node->_db->quote($node->nodeId));

if (!$node->isPermitted('monitor'))
	$query->where("$i.client_node_id = ".$node->_db->quote($synd_user->nodeId));

if (!count($order = tpl_sort_order('issue')))
	$order = array('TS_RESOLVE_BY');

$pager = new PagedListing($node->_storage, $query, SyndLib::array_prepend($order, "$i.", array('SyndLib','isString')), 75);
$issues = SyndLib::filter($pager->getInstances(), 'isPermitted', 'read');

?>
<? if (!empty($issues)) { ?>
	<div class="Result">
		<table style="width:100%;">
			<tr>
				<td>
					<?
					print tpl_text("Displaying %d-%d of %d issues for category <em>'%s'</em>", $pager->getOffset()+1, 
						$pager->getOffset()+count($issues), $pager->getCount(), $node->toString());
					if (min($pager->getCount() - $pager->getOffset(), $pager->getLimit()) > count($issues)) 
						print ' <span class="Info">'.tpl_text('(Some matches are excluded due to access restrictions)').'</span>'; ?><br />
					<? $this->display(tpl_design_path('gui/pager.tpl'),$pager->getParameters()) ?>
				</td>
				<td style="text-align:right; vertical-align:bottom;">
					<? include tpl_design_path('module/issue/issue_status_legend.tpl') ?>
				</td>
			</tr>
		</table>
	</div>
	<? $this->display('model/node/issue/table.tpl', array('list'=>$issues)) ?>
<? } else { ?>
	<em><?= tpl_text('No issues found') ?></em>
<? } ?>
