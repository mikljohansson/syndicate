<? $bodyid = md5(uniqid('')); ?>
<script type="text/javascript">
<!--
	var _display = function(oLink, sId) {
		oLink.blur();

		var oTabs = oLink.parentNode.parentNode.childNodes;
		for (var i=0; i<oTabs.length; i++) {
			if (1 == oTabs.item(i).nodeType) 
				oTabs.item(i).className = 'PaneTab';
		}
		oLink.parentNode.className = 'SelectedPaneTab';
		
		var oBody = document.getElementById('<?= $bodyid ?>');
		for (var i=0; i<oBody.childNodes.length; i++) {
			if (1 == oBody.childNodes.item(i).nodeType) 
				oBody.childNodes.item(i).style.display = 'none';
		}
		document.getElementById(sId).style.display = 'block';
	}
//-->
</script>
<div class="TabbedPane">
	<div class="PaneHeader">
	<? foreach ($tabs as $key => $tab) {
		if ($tab['selected']) { 
			$selected = $key; ?>
		<span class="SelectedPaneTab"><? 
		} else { ?>
		<span class="PaneTab"><? 
		} ?><a href="#" onclick="javascript:_display(this,'<?= $bodyid.'.'.$key ?>');"><?= $tab['text'] ?></a></span> 
	<? } ?>
	</div>
	<div class="PaneBody" id="<?= $bodyid ?>">
		<? 
		if (!isset($selected)) 
			$selected = reset(array_keys($tabs));
		
		foreach ($tabs as $key => $tab) { ?>
			<div id="<?= $bodyid.'.'.$key ?>" style="display:<?= $key == $selected ? 'block' : 'none' ?>;"><?
				if (is_array($tab['template']))
					$this->render($tab['template'][0], $tab['template'][1], $_data);
				else if (is_object($selected['template']))
					print $selected['template'];
				else
					$this->display($tab['template']);
			?></div><?
		}
		?>
	</div>
</div>