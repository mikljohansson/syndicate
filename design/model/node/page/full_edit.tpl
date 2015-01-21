<div class="Article">
	<? $this->render($node,'part_view_header.tpl') ?>
	<br />

	<?
	$pane = clone $this;
	$pane->append('tabs', array(
		'uri' => tpl_link($node->getHandler(),'edit',$node->nodeId),
		'text' => tpl_text('Edit'),
		'template' => array($node,'pane_edit.tpl'),
		'selected' => null == $request[0]));
	$pane->append('tabs', array(
		'uri' => tpl_link($node->getHandler(),'edit',$node->nodeId,'perms'),
		'text' => tpl_text('Permissions'),
		'template' => array($node,'pane_edit_perms.tpl'),
		'selected' => 'perms' == $request[0]));
	$pane->display(tpl_design_path('gui/pane/tabbed.tpl'),
		array('request' => $request, 'data' => $data));
	?>
</div>