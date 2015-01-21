<div class="OpenEndedText">
	<div class="Abstract"><?= $node->getQuestion() ?></div>
	<div class="indent">
		<input type="text" name="<?= $node->getName($attempt) ?>" value="<?= tpl_value($answer) ?>" size="60" maxlength="2000" />
	</div>
	<? $this->render($node,'answer.tpl',$_data) ?>
</div>