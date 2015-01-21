<h1><?= $node->toString() ?></h1>
<? 

$pane = clone $this;
$pane->assign('tabs', array());

$pane->append('tabs', array(
	'uri' => tpl_view($node->getHandler(),'view',$node->nodeId),
	'text' => tpl_text('Info'),
	'template' => array($node,'pane_view.tpl'),
	'selected' => null == $request[0]));

$pane->append('tabs', array(
	'uri' => tpl_view($node->getHandler(),'view',$node->nodeId,'issues'),
	'text' => tpl_text('Issues'),
	'template' => array($node,'pane_view_issues.tpl'),
	'selected' => 'issues' == $request[0]));


if ($node->isPermitted('admin')) {
	$pane->append('tabs', array(
		'uri' => tpl_view($node->getHandler(),'view',$node->nodeId,'admin'),
		'text' => tpl_text('Administer'),
		'template' => array($node,'pane_view_admin.tpl'),
		'selected' => 'admin' == $request[0]));
}

$pane->display('gui/pane/tabbed.tpl');
