<? tpl_load_script(tpl_design_uri('js/ole.js')) ?>
<li><a href="javascript:synd_ole_call('<?= tpl_view_call('course','cut') ?>')"  accesskey="x"
	title="<?= tpl_text('Accesskey: %s','X')
	?>"><?= tpl_text('Cut selected') ?></a></li>
<li><a href="javascript:synd_ole_call('<?= tpl_view_call('course','copy') ?>')"  accesskey="c"
	title="<?= tpl_text('Accesskey: %s','C') 
	?>"><?= tpl_text('Copy selected') ?></a></li>
<li><a href="javascript:synd_ole_call('<?= tpl_view_call('course','delete') ?>')" accesskey="d"
	title="<?= tpl_text('Accesskey: %s','D') 
	?>"><?= tpl_text('Delete selected') ?></a></li>
