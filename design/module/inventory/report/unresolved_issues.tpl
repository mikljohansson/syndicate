<?
$storage = SyndNodeLib::getDefaultStorage('issue');
$database = $storage->getDatabase();

$sql = "
	SELECT i.node_id, i.ts_create FROM synd_issue i, synd_issue_repair r
	WHERE 
		i.node_id = r.node_id AND
		i.info_status < ".$database->quote(synd_node_issue::CLOSED)."
	ORDER BY i.ts_create DESC";
$issues = SyndLib::filter($storage->getInstances($database->getCol($sql)),'isPermitted','read');

?>
<h2><?= tpl_text('Open issues') ?></h2>
<ul class="Actions">
	<li><a href="<?= tpl_link('inventory','report','batch_resolve_issues') ?>">
		<?= tpl_text('Resolve these issues') ?></a></li>
</ul>
<?= tpl_gui_table('repair',$issues,'view_simple.tpl') ?>