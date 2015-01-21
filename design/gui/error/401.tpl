<? 
global $synd_user; 
if (!headers_sent())
	header('HTTP/1.0 401 error: Authorization Required');
$this->assign('title', '401 Authorization Required'); 
?>
<h1>401 Authorization Required</h1>
<p><?= tpl_text('This server could not verify that you are authorized to access the document requested.') ?></p>
<? if (null != $message) { ?>
	<b><?= tpl_text('Note:') ?></b>
	<div style="margin-left:1em;"><?= $message ?></div>
<? } else if ($synd_user->isNull()) { ?>
	<b><?= tpl_text('Note:') ?></b>
	<div style="margin-left:1em;"><?= tpl_text('You might have to login to gain access to this resource.') ?></div>
<? } ?>
<? $this->display(tpl_design_path('gui/error/exception.tpl')) ?>