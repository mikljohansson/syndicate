<div class="MultipleChoice">
	<? 
	$question = $node->getQuestion();
	$options = $node->getOptions();

	foreach ($options as $option) {
		$input = $this->fetchnode($node,'inline_option.tpl',
			array_merge($_data, array('type' => 'radio','option' => $option)));
		$key = preg_quote($option['INFO_OPTION'], '/');
		$question = preg_replace("/<\\(?$key\\)(.*?)>/", $input, $question);
	}
	
	?>
	<div class="Inline">
		<?= $question ?>
		<? if (isset($answer)) { ?>
		<? $this->render($node,'explanation.tpl',$_data) ?>
		<? } ?>
	</div>
</div>