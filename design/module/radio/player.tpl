<style>
	html {
		background-color: #fff;
	}
</style>
<script type="text/javascript">
	function synd_radio_close() {
		var oFrame = window.parent.document.getElementById('synd_radio_main');
		window.parent.location = oFrame.contentWindow.location;
	}
</script>
<img src="<?= tpl_design_uri('module/radio/close.gif') ?>" onclick="synd_radio_close()" style="cursor:pointer; margin:1px;" />
<object style="height:12px; width:500px; margin-top: 1px" height="12" classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000" codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,0,0" id="synd_radio_player" />
	<param name="movie" value="<?= tpl_design_uri('module/radio/player.swf') ?>" />
	<param name="quality" value="high" />
	<param name="bgcolor" value="#ffffff" />
	<embed style="height:12px; width:500px;" height="12" src="<?= tpl_design_uri('module/radio/player.swf') ?>" quality="high" name="synd_radio_player" type="application/x-shockwave-flash" pluginspage="http://www.macromedia.com/go/getflashplayer"></embed>
</object>