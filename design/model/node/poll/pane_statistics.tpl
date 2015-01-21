<div class="Body" style="margin-top:10px; margin-bottom:10px;">
	<h3><?= tpl_text('Statistics summary') ?></h3>
	<div class="indent">
		<?= tpl_text('%d replies to this poll.', $node->getAttemptCount()) ?><br />
	</div>
	<ul class="Actions">
		<li><a href="<?= tpl_link_call($node->getHandler(),'view',$node->nodeId,'attempts') ?>">
			<?= tpl_text('Display individual replies') ?></a></li>
		<li><a href="<?= tpl_link_call($node->getHandler(),'view',$node->nodeId,'replies') ?>">
			<?= tpl_text('Display all replies to Fill-in-the-Blank questions') ?></a></li>
		<li><a href="<?= tpl_link_call($node->getHandler(),'view',$node->nodeId,'emails') ?>">
			<?= tpl_text('Display respondant email addresses') ?></a></li>
	</ul>
	<? $this->render($node,'filters.tpl') ?>
	<h3><?= tpl_text('Statistics per question') ?></h3><hr />
	<? $this->iterate($node->getChildren(),'item_statistics.tpl',
		array('questionNumber' => 1)) ?>
</div>
