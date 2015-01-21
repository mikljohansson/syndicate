<? 
if ($node->isPermitted('write')) 
	SyndLib::attachHook('breadcrumbs', array($node,'_callback_breadcrumbs'));
?>
<div class="Article">
	<div class="Header">
		<h1><?= $node->toString() ?></h1>
	</div>
	<? 
	$pane = clone $this;
	
	$pane->append('tabs', array(
		'uri' => tpl_link($node->getHandler(),$node->objectId()),
		'text' => tpl_text('Poll'),
		'template' => array($node,'pane.tpl'),
		'selected' => null == $request[0]));
	
	if ($node->isPermitted('write')) { 
		$pane->append('tabs', array(
			'uri' => tpl_link($node->getHandler(),'view',$node->nodeId,'design'),
			'text' => tpl_text('Design mode'),
			'template' => array($node,'pane_design.tpl'),
			'selected' => 'design' == $request[0]));
		$pane->append('tabs', array(
			'uri' => tpl_link($node->getHandler(),'view',$node->nodeId,'admin'),
			'text' => tpl_text('Administrera'),
			'template' => array($node,'pane_admin.tpl'),
			'selected' => 'admin' == $request[0]));
	}

	if ($node->isPermitted('statistics')) { 
		$pane->append('tabs', array(
			'uri' => tpl_link($node->getHandler(),'view',$node->nodeId,'statistics'),
			'text' => tpl_text('Statistics'),
			'template' => array($node,'pane_statistics.tpl'),
			'selected' => 'statistics' == $request[0]));
	}
	
	$pane->display(tpl_design_path('gui/pane/tabbed_header.tpl'),
		array('request' => $request));
	?>
</div>