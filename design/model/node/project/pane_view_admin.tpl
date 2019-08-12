<? global $synd_user; ?>
<ul class="Actions">
	<li><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'newProject') 
		?>"><?= tpl_text('Create subproject to %s', $node->toString()) ?></a></li>
	<li><a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) 
		?>"><?= tpl_text('Edit this project') ?></a></li>
</ul>
<br />

<?
$pane = clone $this;
$pane->assign('tabs', array());
$pane->assign('request', $request->forward());

$pane->append('tabs', array(
	'uri' => tpl_link($node->getHandler(),'project',$node->nodeId,'admin'),
	'text' => tpl_text('Info'),
	'template' => array($node,'pane_view_admin_info.tpl'),
	'selected' => null == $request[0]));

$pane->append('tabs', array(
	'uri' => tpl_link($node->getHandler(),'project',$node->nodeId,'admin','keywords'),
	'text' => tpl_text('Categories'),
	'template' => array($node,'pane_view_admin_keywords.tpl'),
	'selected' => 'keywords' == $request[0]));

$pane->append('tabs', array(
	'uri' => tpl_link($node->getHandler(),'project',$node->nodeId,'admin','perms'),
	'text' => tpl_text('Permissions'),
	'template' => array($node,'pane_view_admin_perms.tpl'),
	'selected' => 'perms' == $request[0]));

$pane->append('tabs', array(
	'uri' => tpl_link($node->getHandler(),'project',$node->nodeId,'admin','mappings'),
	'text' => tpl_text('Address Mappings'),
	'template' => array($node,'pane_view_admin_mappings.tpl'),
	'selected' => 'mappings' == $request[0]));

SyndLib::runHook('project_admin', $node, $request, $pane);

$pane->display('gui/pane/tabbed.tpl');
