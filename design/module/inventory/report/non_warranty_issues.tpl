<?
$storage = SyndNodeLib::getDefaultStorage('issue');
$database = $storage->getDatabase();

$sql = "
	SELECT COUNT(1) FROM synd_issue i, synd_issue_repair r
	WHERE 
		i.node_id = r.node_id AND
		i.info_status < ".$database->quote(synd_node_issue::CLOSED)." AND
		r.flag_no_warranty = 1";
$count = $database->getOne($sql);

$sql = "
	SELECT i.node_id, i.ts_create FROM synd_issue i, synd_issue_repair r
	WHERE 
		i.node_id = r.node_id AND
		i.info_status < ".$database->quote(synd_node_issue::CLOSED)." AND
		r.flag_no_warranty = 1
	ORDER BY i.ts_create DESC";

$limit = 40;
$offset = isset($_REQUEST['offset']) ? $_REQUEST['offset'] : 0;
$issues = $storage->getInstances($database->getCol($sql,0,$offset,$limit));

$this->assign('limit', $limit);
$this->assign('offset', $offset);
$this->assign('count', $count);

?>
<h3><?= tpl_text('Non warranty issues') ?></h3>
<div class="indent">
	<? if ($count) { ?>
		<? $this->display(tpl_design_path('gui/pager.tpl')) ?>
		<? $this->display('model/node/issue/table.tpl',array('list'=>$issues)) ?>
	<? } else { ?>
		<em><?= tpl_text('No non warranty repairs at the moment.') ?></em>
	<? } ?>
</div>