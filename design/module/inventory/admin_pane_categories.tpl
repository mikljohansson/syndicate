<? 
$storage = SyndNodeLib::getDefaultStorage('class');
$database = $storage->getDatabase();

$sql = "
	SELECT c.node_id FROM synd_class c
	ORDER BY c.name";
$classes = $storage->getInstances($database->getCol($sql));
?>
<ul class="Actions">
	<li><a href="<?= tpl_link_call('inventory','newCategory') ?>"><?= tpl_text('Create new category') ?></a></li>
</ul>

<? $this->iterate($classes,'item.tpl') ?>