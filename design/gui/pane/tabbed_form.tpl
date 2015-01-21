<div class="TabbedPane">
	<input type="hidden" name="redirect" id="<?= $id=md5(uniqid('')) ?>" value="" />
	<div class="PaneHeader">
	<? foreach ($tabs as $tab) {
		if ($tab['selected']) { 
			$selected = $tab;
			?><input class="SelectedPaneTab"<?
		} else { 
			?><input class="PaneTab"<?
		} 
		?> type="button" onclick="document.getElementById('<?= $id ?>').value='<?= $tab['uri'] ?>'; this.form.submit();" value="<?= $tab['text'] ?>" />
	<? } ?>
	</div>
	<? include tpl_design_path('gui/pane/tabbed_body.tpl') ?>
</div>