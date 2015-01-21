<div class="OpenEndedText">
	<?
	$name = $node->getName($attempt);
	$input = "<input type=\"text\" name=\"$name\" value=\"$answer\" size=\"\\1\" tabindex=\"1\" />";
	if (isset($answer)) 
		$input .= $this->fetchnode($node,'explanation.tpl',$_data);
	$question = preg_replace('/<(\d+)>/', $input, $node->getQuestion());

	?>
	<?= $question ?>
	<? if (isset($answer)) { ?>
	<div class="Answer">
		<? 
		$correct = preg_replace('/<(.+?\/.+?)>/', '\1', $node->getCorrectAnswer());
		$correct = "<span class=\"Correct\">$correct</span>";
		print preg_replace('/<(\d+)>/', $correct, $node->getQuestion());
		?>
	</div>
	<? } ?>
</div>