	<div class="Body" style="margin-top:10px; margin-bottom:10px;">
		<? $content = $node->getContent(); print $content->toString(); ?>

		<hr />
		<? if (isset($request['attempt']) && null != ($attempt = SyndNodeLib::getInstance($request['attempt'])) && $attempt->isPermitted('read')) { ?>
			<div class="Notice">
				<?= tpl_text('You are now viewing the results from:') ?>
				<? $this->render($attempt, 'info.tpl') ?>
			</div>
			<? $this->iterate($node->getChildren(),'item.tpl', array('attempt' => $attempt, 'questionNumber' => 1)) ?>
		<? } else { ?>
		<form action="<?= tpl_uri_merge(null,tpl_link($node->getHandler(),'invoke',$node->nodeId,'submit')) ?>" method="post">
			<? $attempt = $node->createAttempt(); ?>
			<? $this->iterate($node->getChildren(),'item.tpl',
				array('attempt' => $attempt, 'questionNumber' => 1)) ?>
			<input type="submit" value="<?= tpl_text('Send my answers') ?>" />
		</form>
		<? } ?>		
	</div>