<div class="Article">
	<? $this->render($node,'part_view_header.tpl') ?>
	<br />

	<?
	$pane = clone $this;
	$pane->append('tabs', array(
		'uri' => tpl_view($node->getHandler(),'view',$node->nodeId,'admin'),
		'text' => tpl_text('Groups'),
		'template' => array($node,'pane_admin_groups.tpl'),
		'selected' => null == $request[0] || 'groups' == $request[0]));
	$pane->append('tabs', array(
		'uri' => tpl_view($node->getHandler(),'view',$node->nodeId,'admin','perms'),
		'text' => tpl_text('Permissions'),
		'template' => array($node,'pane_admin_perms.tpl'),
		'selected' => 'perms' == $request[0]));
	$pane->append('tabs', array(
		'uri' => tpl_view($node->getHandler(),'view',$node->nodeId,'admin','stylesheet'),
		'text' => tpl_text('CSS Stylesheet'),
		'template' => array($node,'pane_admin_stylesheet.tpl'),
		'selected' => 'stylesheet' == $request[0]));
	$pane->display(tpl_design_path('gui/pane/tabbed.tpl'),
		array('request' => $request->forward()));
	?>
</div>