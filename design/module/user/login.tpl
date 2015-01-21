<?
function _callback_html_head_title_login(&$result) {
	return $result = tpl_text('Login to %s', $_SERVER['SERVER_NAME']);
}
SyndLib::attachHook('html_head_title', '_callback_html_head_title_login');
$module = Module::getInstance('user');

?>
<form action="<?= tpl_link_jump('user','login') ?>" method="post">
	<h4><?= tpl_text('Username') ?></h4>
	<input type="text" name="username" id="username" value="<?= $request['username'] ?>" size="12" />

	<h4><?= tpl_text('Password') ?></h4>
	<input type="password" name="password" size="12" />

	<div style="margin-bottom:1em;">
		<input type="checkbox" name="persist" id="persist" value="1" />
			<label for="persist" title="<?= tpl_text('Remember login from this computer') ?>"><?= tpl_text('Remember me') ?></label>
	</div>

	<input type="submit" value="<?= tpl_text('Login') ?>" />

	<ul class="Actions" style="margin-top:2em;">
		<li><a href="<?= tpl_link('user','password') ?>"><?= tpl_text('Forgot your password?') ?></a></li>
		<? if ($module->isPermitted('signup')) { ?>
		<li><a href="<?= tpl_link('user','signup') ?>"><?= tpl_text('Make a new account') ?></a></li>
		<? } ?>
	</ul>
</form>
<script type="text/javascript">
<!--
	if (document.getElementById && null != document.getElementById('username')) 
		document.getElementById('username').focus();
//-->
</script>