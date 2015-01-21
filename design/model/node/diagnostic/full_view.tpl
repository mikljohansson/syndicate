<div class="Article">
	<? $this->render($node,'part_view_header.tpl') ?>
	<br />
	<? 
	$pane = clone $this;
	
	$pane->append('tabs', array(
		'uri' => tpl_link($node->getHandler(),'view',$node->nodeId),
		'text' => tpl_text('Diagnostic test'),
		'template' => array($node,'pane.tpl'),
		'selected' => null == $request[0]));
		
	if ($node->isPermitted('write')) { 
		$pane->append('tabs', array(
			'uri' => tpl_link($node->getHandler(),'view',$node->nodeId,'design'),
			'text' => tpl_text('Design mode'),
			'template' => array($node,'pane_design.tpl'),
			'selected' => 'design' == $request[0]));
	}
	
	$pane->display(tpl_design_path('gui/pane/tabbed_header.tpl'),
		array('request' => $request));
	?>

	<? $this->render($node,'part_view_footer.tpl') ?>
</div>
