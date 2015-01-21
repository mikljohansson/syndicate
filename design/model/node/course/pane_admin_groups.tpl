<? 
tpl_load_script(tpl_design_uri('js/ole.js'));
$courseModule = Module::getInstance('course');
if (!isset($request[0]) || null == ($group = SyndNodeLib::getInstance($request[0])))
	$group = $node->getGroup();

$branch = array();
$current = $group;
while (!($current->getParent() instanceof synd_node_course)) {
	$branch[] = $current;
	$current = $current->getParent();
}

?>
<div style="margin-bottom:5px;">
	<a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId,'admin') ?>"><?= $node->toString() ?></a>
	<? foreach (array_reverse(array_keys($branch)) as $key) { ?>
	&raquo; <a href="<?= tpl_link($node->getHandler(),'view',
		$node->nodeId,'admin','groups',$branch[$key]->nodeId) ?>"><?= $branch[$key]->toString() ?></a>
	<? } ?>
<? 

if ('email' == $request[1]) {
	// Send email to group
	$pane = $this->fetchnode($node,'part_view_email.tpl',
		array('group' => $group, 'request' => $request));
}
else if ('create_group' == $request[1]) {
	// Create new subgroup pane
	$pane = $this->fetchnode($node,'part_view_create_group.tpl',
		array('group' => $group, 'request' => $request));
}
else if ('create_clients' == $request[1]) {
	// Create client accounts pane
	$pane = $this->fetchnode($node,'part_view_create_clients.tpl',
		array('group' => $group, 'request' => $request));
}
else if (isset($request[1]) && null != ($client = SyndNodeLib::getInstance($request[1]))) {
	// Display client information
	?> &raquo; <a href="<?= tpl_link($node->getHandler(),'view',
		$node->nodeId,'admin','groups',$group->nodeId,$client->nodeId) ?>"><?= $client->toString() ?></a><?
	$pane = $this->fetchnode($node,'part_view_client.tpl',
		array('group' => $group, 'client' => $client, 'request' => $request));
	$courseModule->setOleTarget($client);
}
else {
	// Display subgroups and clients (default)
	$pane = $this->fetchnode($node,'part_view_groups.tpl',
		array('group' => $group, 'request' => $request));
	$courseModule->setOleTarget($group);
}

?>
</div>

<?= $pane ?>