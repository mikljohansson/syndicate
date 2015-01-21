<div class="Item">
	<h2><?= $node->toString() ?></h2>
	<? 
	$answers = $node->getAnswers();
	$texts = array_filter(SyndLib::array_collect($answers, 'INFO_ANSWER'));
	$length = empty($texts) ? 0 : array_sum(array_map('strlen', $texts)) / count($texts);
	
	foreach ($texts as $i => $answer) { 
		if ($length <= 4) {
			print $answer;
			if (isset($texts[$i+1]))
				print ', ';
		}
		else {
	?>
	<div style="margin-top:0.5em;">
		<?= $answer ?>
	</div>
	<? 
		}
	} 
	?>
</div>
<hr />