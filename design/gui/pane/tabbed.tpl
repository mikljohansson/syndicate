<div class="TabbedPane">
	<div class="PaneHeader">
	<? foreach ($tabs as $tab) {
		if ($tab['selected']) { 
			$selected = $tab;
			?><span class="SelectedPaneTab"><?
		} else { 
			?><span class="PaneTab"><?
		} 
		?><a href="<?= $tab['uri'] ?>"><?= $tab['text'] ?></a></span> 
	<? } ?>
	</div>
	<? include tpl_design_path('gui/pane/tabbed_body.tpl') ?>
</div>