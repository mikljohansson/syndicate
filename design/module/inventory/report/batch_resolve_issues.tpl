<?
$storage = SyndNodeLib::getDefaultStorage('issue');
$database = $storage->getDatabase();
$inventory = Module::getInstance('inventory');

$sql = "
	SELECT i.node_id, i.ts_create FROM synd_issue i, synd_issue_repair r
	WHERE 
		i.node_id = r.node_id AND
		i.info_status < ".$database->quote(synd_node_issue::CLOSED)."
	ORDER BY i.ts_create DESC";
$issues = SyndLib::filter($storage->getInstances($database->getCol($sql)),'isPermitted','read');

?>
<h1><?= tpl_text('Open issues') ?></h1>
<div class="Info">
	<p><?= tpl_text('Use when handing a repaired item back to the client or accepting a batch of items back from external repairs.') ?></p>
	<ul>
		<li><?= tpl_text('Type in all supplementary details, such as cause of error and measures taken, when resolving a repair issue.') ?></li>
		<? if (null != ($folder = $inventory->getRepairedFolder())) { ?>
		<li><?= tpl_translate('Place the items on the %s shelf if they are back from external repair, otherwise hand them back to their client.', $this->fetchnode($folder,'head_view.tpl')) ?></li>
		<? } ?>
	</ul>
</div>
<br />

<? if (count($issues)) { ?>
<form action="<?= tpl_view_call('system','mplex') ?>" method="post">
	<? $this->display('model/node/repair/table_edit.tpl',array('list'=>$issues)) ?>
	<span title="<?= tpl_text('Accesskey: %s','S') ?>">
		<input accesskey="s" type="submit" value="<?= tpl_text('Save and resolve') ?>" />
	</span>
</form>
<? } else { ?>
	<em><?= tpl_text('No active issues found.') ?></em>
<? } ?>
