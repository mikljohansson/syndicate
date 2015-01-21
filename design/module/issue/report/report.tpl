<? 
tpl_load_script(tpl_design_uri('js/json.js'));
tpl_load_script(tpl_design_uri('js/autocomplete.js'));
tpl_load_script(tpl_design_uri('module/issue/issue.js'));

?>
<h1><?= tpl_text('Issue report') ?></h1>
<form class="Report" action="<?= tpl_link('issue','report') ?>" method="get">
	<?
	$this->append('tabs', array(
		'text' => tpl_text('Filters'),
		'template' => tpl_design_path('module/issue/report/report_pane_default.tpl'),
		'selected' => true));
	$this->append('tabs', array(
		'text' => tpl_text('Details'),
		'template' => tpl_design_path('module/issue/report/report_pane_details.tpl')));
	$this->display(tpl_design_path('gui/pane/tabbed_javascript.tpl'));
	?>
	<p>
		<input type="submit" name="output[html]" value="<?= tpl_text('Display') ?>" />
		<input type="submit" name="output[xls]" value="<?= tpl_text('Download (Excel)') ?>" />
	</p>
</form>
<script type="text/javascript">
<!--
	if (document.getElementById) {
		window.onload = function() {
			new AutoComplete(document.getElementById('customer'), '<?= tpl_view('rpc','json','issue') ?>', 'findSuggestedUsers');
			new AutoComplete(document.getElementById('organisation'), '<?= tpl_view('rpc','json','issue') ?>', 'findSuggestedRoles');
			new AutoComplete(document.getElementById('creator'), '<?= tpl_view('rpc','json','issue') ?>', 'findSuggestedUsers');
			new AutoComplete(document.getElementById('updater'), '<?= tpl_view('rpc','json','issue') ?>', 'findSuggestedUsers');
			new AutoComplete(document.getElementById('commenter'), '<?= tpl_view('rpc','json','issue') ?>', 'findSuggestedUsers');
		};
	}
//-->
</script>

<? if (isset($report)) { ?>
<? $this->display(tpl_design_path("module/issue/report/formats/$format.html.tpl")) ?>
<? } ?>
