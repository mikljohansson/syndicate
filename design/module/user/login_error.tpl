<?
function _callback_html_head_title_login_error(&$result) {
	return $result = tpl_text('401 Authorization Required');
}
SyndLib::attachHook('html_head_title', '_callback_html_head_title_login_error');
?>
<h1>401 Authorization Required</h1>
<p class="Warning"><?= $message ?></p>