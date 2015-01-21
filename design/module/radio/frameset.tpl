<frameset rows="14,*">
	<frame id="synd_radio_frame" src="<?= tpl_view('radio','player') ?>" 
		frameborder="0" marginwidth="0" marginheight="0" scrolling="no" style="padding-top:1px;" />
	<frame id="synd_radio_main" src="<?= tpl_value($_COOKIE['synd']['radio']['page'],'/') ?>" 
		frameborder="0" marginwidth="0" marginheight="0" scrolling="auto" style="border-top:1px solid black;" />
</frameset>