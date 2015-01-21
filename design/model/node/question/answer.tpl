<? if (isset($answer)) { ?>
<div class="Answer">
	<? $this->render($node,'explanation.tpl',$_data) ?>
	<?= htmlspecialchars($node->data['INFO_CORRECT_ANSWER']) ?>
</div>
<? } ?>