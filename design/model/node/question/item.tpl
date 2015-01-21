<?
$class = 'Item';
$answer = $attempt->getAnswer($node);
$answered = false;

if (is_array($answer)) {
	if (count($answer = array_filter($answer))) {
		$class .= $node->isCorrect($answer) ? ' Correct' : ' Incorrect';
		$answered = true;
	}
}
else if (null !== $answer && '' !== $answer) {
	$class .= $node->isCorrect($answer) ? ' Correct' : ' Incorrect';
	$answered = true;
}

?>
<? if (isset($questionNumber)) { ?>
<table style="width:100%">
<tr>
	<td style="padding-right:5px; width:20px;"><b><?= $questionNumber++ ?>:</b></td>
	<td class="<?= $class ?>">
	<? } ?>
		<? $this->render($node,'layout/'.$node->getLayout().'.tpl',
			!empty($answer) ? array_merge($_data, array('answer'=>$answer)) : $_data) ?>
<? if (isset($questionNumber)) { ?>
	</td>
</tr>
</table>
<? } ?>