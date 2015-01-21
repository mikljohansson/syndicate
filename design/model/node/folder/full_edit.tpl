<div class="Article">
	<div class="Header">
		<h1><?= $node->toString() ?></h1>
		<? if (null != $node->data['INFO_DESC']) { ?>
		<div class="Info"><?= tpl_text($node->data['INFO_DESC']) ?></div>
		<? } ?>
	</div>
	<? include tpl_design_path('gui/errors.tpl'); ?>

	<?
	$this->append('tabs', array(
		'uri' => tpl_view($node->getHandler(),'edit',$node->nodeId),
		'text' => tpl_text('Content'),
		'template' => array($node,'pane_edit.tpl'),
		'selected' => null == $request[0]));

	$this->append('tabs', array(
		'uri' => tpl_view($node->getHandler(),'edit',$node->nodeId,'config'),
		'text' => tpl_text('Configuration'),
		'template' => array($node,'pane_edit_config.tpl'),
		'selected' => 'config' == $request[0]));

	$this->display(tpl_design_path('gui/pane/tabbed_form.tpl'),$_data);
	?>
</div>