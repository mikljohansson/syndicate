<? 
if (!headers_sent())
	header('HTTP/1.0 404 error: Not Found');
$this->assign('title', '404 Not Found'); 
?>
<h1>404 Not Found</h1>
<p><?= tpl_text('The requested URL was not found on this server.') ?></p>
<? if (null != $message) { ?>
	<b><?= tpl_text('Note:') ?></b>
	<div style="margin-left:1em;"><?= $message ?></div>
<? } ?>
<?= Module::runHook('http_error_message', $status, $uri) ?>
<? $this->display(tpl_design_path('gui/error/exception.tpl')) ?>
