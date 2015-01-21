<div class="OpenEndedText">
	<? if (isset($answer)) { ?>
	<?= tpl_form_textarea($node->getName($attempt),$answer,
		array('cols'=>72)) ?>
	<? $this->render($node,'answer.tpl',$_data) ?>
	<? } else { ?>
	<?= tpl_form_textarea($node->getName($attempt),$node->getQuestion(),
		array('cols'=>72)) ?>
	<? } ?>
</div>