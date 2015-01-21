<div style="margin-bottom:1em;">
	<a href="<?= tpl_link($node->getHandler()) ?>"><?= tpl_text('Home') ?></a> &raquo;
	<a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>"><?= $node->toString() ?></a>
</div>

<div class="indent">
	<?= $node->toString() ?>
</div>
<br />

<? 
$page = $node->getPage();
$sql = "
	SELECT DISTINCT a.client_node_id 
	FROM synd_attempt a
	WHERE a.parent_node_id = ".$page->_db->quote($page->nodeId);
$clients = SyndNodeLib::getInstances($page->_db->getCol($sql));

print implode('<br />', array_filter(SyndLib::invoke($clients,'getEmail')));

