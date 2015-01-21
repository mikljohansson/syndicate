<h1><?= $node->getTitle() ?></h1>
<? 

include tpl_gui_path('synd_node_issue','confirm_email_status.tpl');
$pane = clone $this;
$pane->assign('tabs', array());

if ('archived' == $request[0]) {
	$pane->append('tabs', array(
		'uri' => tpl_view('issue','project',$node->getProjectId()),
		'text' => tpl_text('Info'),
		'template' => array($node,'pane_view_archived.tpl'),
		'selected' => 'archived' == $request[0]));
}
else {
	$pane->append('tabs', array(
		'uri' => tpl_view('issue','project',$node->getProjectId()),
		'text' => tpl_text('Info'),
		'template' => array($node,'pane_view.tpl'),
		'selected' => null == $request[0]));
}

$pane->append('tabs', array(
	'uri' => tpl_view('issue','project',$node->getProjectId(),'issues'),
	'text' => tpl_text('Issues'),
	'partial' => array(tpl_uri_merge(null, 'issue/project/'.$node->getProjectId().'/issues/'), 'issues'),
	'template' => array($node,'pane_view_issues.tpl'),
	'selected' => 'issues' == $request[0] || 'recent' == $request[0]));

if ($node->isPermitted('write')) {
	$pane->append('tabs', array(
		'uri' => tpl_view('issue','project',$node->getProjectId(),'notification'),
		'text' => tpl_text('Notification'),
		'template' => array($node,'pane_view_notification.tpl'),
		'selected' => 'notification' == $request[0]));
}

if ($node->isPermitted('admin')) {
	$pane->append('tabs', array(
		'uri' => tpl_view('issue','project',$node->getProjectId(),'admin'),
		'text' => tpl_text('Administer'),
		'template' => array($node,'pane_view_admin.tpl'),
		'selected' => 'admin' == $request[0]));
}

$pane->display('gui/pane/tabbed.tpl');
