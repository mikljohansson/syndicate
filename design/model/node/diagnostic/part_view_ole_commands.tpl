<? tpl_load_script(tpl_design_uri('js/ole.js')) ?>
<li><a href="javascript:synd_ole_call('<?= tpl_view_call('course','cut') ?>')" 
	title="<?= tpl_text('Cut the selected chapters and questionnaires from this page') 
	?>"><?= tpl_text('Cut selected') ?></a></li>
<li><a href="javascript:synd_ole_call('<?= tpl_view_call('course','copy') ?>')" 
	title="<?= tpl_text('Copy the selected chapters and questionnaires') 
	?>"><?= tpl_text('Copy selected') ?></a></li>
<li><a href="javascript:synd_ole_call('<?= tpl_view_call('course','invoke',$node->nodeId,'remove') ?>')" 
	title="<?= tpl_text('Remove the selected questions from this diagnostic test') 
	?>"><?= tpl_text('Remove selected from test') ?></a></li>
