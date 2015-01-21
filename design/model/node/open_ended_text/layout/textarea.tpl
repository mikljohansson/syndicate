<div class="OpenEndedText">
	<div class="Abstract"><?= $node->getQuestion() ?></div>
	<?= tpl_form_textarea($node->getName($attempt),$answer,
		array('cols'=>72)) ?>
	<? $this->render($node,'answer.tpl',$_data) ?>
</div>