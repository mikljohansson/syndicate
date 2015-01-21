<div class="Success">
	<h2><?= tpl_text('Thank you for your feedback') ?></h2>
	<p><?= tpl_text('Your feedback is valuable to us and we rely on it to help improve the quality of our service and maximize our customer satisfaction. We shall ensure that your feedback is used only in the overall improvement of our service.') ?></p>
	<? if (!$feedback) { ?>
	<form action="<?= tpl_link($node->getHandler(),'invoke',$node->nodeId,'reopen',$node->getAuthenticationToken()) ?>" method="post">
		<h3><?= tpl_text('Option to reopen issue') ?></h3>
		<p><?= tpl_text('You might choose to reopen the issue if you for some reason are not satisfied with the provided solution and provide an optional message in the box below.') ?></p>
		<h4><?= tpl_text('Feedback or message') ?></h4>
		<textarea name="note" cols="76" rows="3"></textarea>
		<p><input type="submit" value="<?= tpl_text('Reopen issue') ?>" /></p>
	</form>
	<? } ?>
</div>