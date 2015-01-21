<div class="Article" style="width:600px;">
	<h1><?= $node->toString() ?></h1>
	<? include tpl_design_path('gui/errors.tpl'); ?>
	
	<?
	
	$pane = clone $this;
	$pane->append('tabs', array(
		'uri' => tpl_view($node->getHandler(),'edit',$node->nodeId),
		'text' => tpl_text('Info'),
		'template' => array($node,'pane_edit_default.tpl'),
		'selected' => null == $request[0]));
	
	if (!count($locales = SyndLib::runHook('getlocales')))
		$locales = array('en' => 'English');
	foreach ($locales as $locale => $info) {
		$pane->append('tabs', array(
			'uri' => tpl_view($node->getHandler(),'edit',$node->nodeId,'templates',$locale),
			'text' => tpl_text('Templates (%s)', $info['name']),
			'template' => array($node,'pane_edit_templates.tpl'),
			'selected' => 'templates' == $request[0] && $locale == $request[1]));
	}

	$pane->display(tpl_design_path('gui/pane/tabbed_form.tpl'),$_data);
	?>

	<p>
		<input accesskey="s" title="<?= tpl_text('Accesskey: %s','S') ?>" type="submit" name="post" value="<?= tpl_text('Save') ?>" />
		<input accesskey="a" title="<?= tpl_text('Accesskey: %s','A') ?>" type="button" value="<?= tpl_text('Abort') ?>"  onclick="window.location='<?= tpl_uri_return() ?>';" />
		<? if (!$node->isNew()) { ?>
		<input type="button" value="<?= tpl_text('Delete') ?>" onclick="window.location='<?= 
			tpl_view_call('issue','delete',$node->nodeId,array('redirect'=>
			$node->getParent()->isNull() ? tpl_view('issue') : tpl_view('issue','project',$node->getParent()->getProjectId()))) ?>';" />
		<? } ?>
	</p>
</div>
