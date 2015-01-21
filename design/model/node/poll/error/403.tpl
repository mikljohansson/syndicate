<? 
global $synd_user; 
if (!headers_sent())
	header('HTTP/1.0 403 error: Forbidden');
function _callback_html_head_title_error(&$result) {return $result = '403 Forbidden';}
SyndLib::attachHook('html_head_title', '_callback_html_head_title_error');
?>
<div class="Article">
	<div class="Notice">
		<?= tpl_text('You do not have permission to view this poll, please contact your systems administrator if the problem persists.') ?>
		<? if ($synd_user->isNull()) { ?><?= tpl_text('As you are not logged in, you might have to do so in order to gain access to this resource.') ?><? } ?>
	</div>

	<div class="Header">
		<h2><?= $node->toString() ?></h2>
	</div>
	<? if (null != $node->getDescription()) { ?>
	<div class="Abstract">
		<?= $node->getDescription() ?>
	</div>
	<? } ?>

	<? $error = $node->getErrorDocument(); ?>
	<?= $error->getText() ?>
</div>
