<? 
global $synd_user; 
tpl_load_script(tpl_design_uri('js/json.js'));
tpl_load_script(tpl_design_uri('js/autocomplete.js'));

?>
<h1><?= tpl_text('User administration') ?></h1>
<form action="<?= tpl_link_call('user','su') ?>" method="post">
	<? include tpl_design_path('gui/errors.tpl'); ?>
	<div class="RequiredField<? if (isset($errors['username'])) print ' InvalidField'; ?>">
		<h3><?= tpl_text('Username') ?></h3>
		<input type="text" name="username" id="username" value="<?= $request['username'] ?>" size="40" />
		<script type="text/javascript">
		<!--
			if (document.getElementById) {
				window.onload = function() {
					new AutoComplete(document.getElementById('username'), '<?= tpl_view('rpc','json','user') ?>', 'findSuggestedUsers', true);
					document.getElementById('username').focus();
				};
			}
		//-->
		</script>
	</div>
	<p>
		<input class="button" type="submit" value="<?= tpl_text('Set') ?>" />
		<input class="button" type="button" value="<?= tpl_text('Abort') 
			?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
		<? if (isset($real)) { ?>
		<input class="button" type="button" value="<?= tpl_text('Reset') 
			?>" onclick="window.location='<?= tpl_link_jump('user','su',array('reset'=>1)) ?>';" />
		<? } ?>
	</p>
</form>	
<? if (isset($real)) { ?>
<p><?= tpl_translate('Effective user is \'<a href="%s">%s</a>\'', tpl_link('user','summary',$synd_user->nodeId), tpl_quote($synd_user->toString())) ?></p>
<? } ?>
