<?
global $synd_user;
require_once 'core/db/SyndDBLib.class.inc';
require_once 'core/model/node/issue.class.inc';

$storage = SyndNodeLib::getDefaultStorage('invoice');
$query = synd_node_invoice::getEntityQuery($storage);

$issueModule = Module::getInstance('issue');
$projects = $issueModule->getProjectTree(new MethodDecider('hasPermission', array($synd_user, 'read')), $synd_user);
$projects->rewind();

$limit = 50;
$offset = isset($_REQUEST['offset']) ? $_REQUEST['offset'] : 0;

if ($projects->valid()) {
	$i = $query->join('synd_issue');
	$v = $query->join('synd_issue_invoice');
	
	//$query->in("$i.parent_node_id", new MemberIterator($projects, 'nodeId'));
	$query->where("$i.info_status >= 0 AND $i.info_status < ".synd_node_issue::CLOSED);
	$query->where("$v.ts_paid IS NULL");
			
	if (isset($request['query']) && '' != $request['query'])
		$query->where(SyndDBLib::sqlLikeExpr($request['query'], array("$i.info_head","$i.data_content","$v.info_amount_untaxed","$v.info_amount_taxed")));
	
	if (!empty($request['from']) && -1 != ($ts = strtotime($request['from'])))
		$query->where("$i.ts_resolve_by >= ".strtotime('00:00:00', $ts));
	if (!empty($request['to']) && -1 != ($ts = strtotime($request['to'])))
		$query->where("$i.ts_resolve_by <= ".strtotime('23:59:59', $ts));

	$order = (array)tpl_sort_order('invoice');
	foreach ($order as $i => $column) {
		if (is_string($column) && !is_numeric($column) && !empty($column)) {
			$query->column($column);
			$query->order($column, !isset($order[$i+1]) || !empty($order[$i+1]));
		}
	}


	$entities = $query->getEntities();
	$count = $entities->count();
	$invoices = iterator_to_array($entities->getIterator($offset, $limit));
}
else {
	$count = 0;
	$invoices = array();
}

?>

<form action="<?= tpl_link('inventory','report','invoices') ?>" method="get">
	<div class="OptionalField">
		<h3><?= tpl_text('Text filter') ?></h3>
		<input type="text" name="query" value="<?= tpl_attribute($request['query']) ?>" size="64" />
	</div>
	<div class="OptionalField">
		<h3><?= tpl_text('Due date between') ?></h3>
		<input type="text" name="from" value="<?= tpl_attribute($request['from']) ?>" size="10" /> <?= tpl_text('and') ?>
		<input type="text" name="to" value="<?= tpl_attribute($request['to']) ?>" size="10" /> <span class="Info">(YYYY-MM-DD)</span>
	</div>
	<input type="submit" value="<?= tpl_text('Update') ?>" />
</form>

<div class="Result">
	<?= tpl_text("Results %d-%d of %d unpaid invoices", $offset+1, $offset+count($invoices), $count) ?>
	<? $this->display(tpl_design_path('gui/pager.tpl'),array('limit'=>$limit,'offset'=>$offset,'count'=>$count)); ?>
</div>
<? $this->display('model/node/invoice/table.tpl',array('list'=>$invoices,'collection'=>$collection)) ?>
