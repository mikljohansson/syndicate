<form action="<?= tpl_link_jump('user','login') ?>" method="post">
	<h4><?= tpl_text('Username') ?></h4>
	<input type="text" name="username" id="username" size="12" />
	<h4><?= tpl_text('Password') ?></h4>
	<input type="password" name="password" size="12" autocomplete="off" />
	<p><input type="submit" value="<?= tpl_text('Login') ?>" /></p>
</form>
<? if ($module->isPermitted('signup') || $module->isPermitted('recover_password')) { ?>
<ul class="Actions">
	<? if ($module->isPermitted('signup')) { ?>
	<li><a href="<?= tpl_link('user','signup') ?>"><?= tpl_text('Create new account') ?></a></li>
	<? } ?>
	<? if ($module->isPermitted('recover_password')) { ?>
	<li><a href="<?= tpl_link('user','password') ?>"><?= tpl_text('Request new password') ?></a></li>
	<? } ?>
<ul>
<? } ?>
<script type="text/javascript">
<!--
	if (document.getElementById) 
		document.getElementById('username').focus();
//-->
</script>