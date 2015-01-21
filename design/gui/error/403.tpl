<? 
global $synd_user; 
if (!headers_sent())
	header('HTTP/1.0 403 error: Forbidden');
$this->assign('title', '403 Forbidden'); 
?>
<h1>403 Forbidden</h1>
<p><?= tpl_text('You don\'t have permission to access this URL.') ?></p>
<? if (null != $message) { ?>
	<b><?= tpl_text('Note:') ?></b>
	<div style="margin-left:1em;"><?= $message ?></div>
<? } else if ($synd_user->isNull()) { ?>
	<b><?= tpl_text('Note:') ?></b>
	<div class="indent"><?= tpl_text('You might have to login to gain access to this resource.') ?></div>
<? } ?>
<? $this->display(tpl_design_path('gui/error/exception.tpl')) ?>
