<input type="hidden" name="collections[]" value="<?= $node->id() ?>" />
<div class="Article">
	<div class="Header">
		<h1><?= $node->toString() ?></h1>
	</div>
	<?
	$this->append('tabs', array(
		'uri' => tpl_view('issue','keyword',$node->nodeId),
		'text' => tpl_text('Issues'),
		'template' => tpl_design_path('module/issue/keyword_pane_issues.tpl'),
		'selected' => null == $request[0]));

	$this->display(tpl_design_path('gui/pane/tabbed.tpl'));
	?>
</div>
