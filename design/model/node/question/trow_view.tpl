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
<tr>
	<? if (isset($questionNumber)) { ?>
	<td style="padding-top:10px;"><b><?= $questionNumber++ ?>:</b></td>
	<? } ?>
	<td class="<?= $class ?>" style="padding:8px;">
		<? $this->render($node,'layout/'.$node->getLayout().'.tpl',
			$answered ? array_merge($_data, array('answer'=>$answer)) : $_data) ?>
	</td>
	<? if ($node->isPermitted('write')) { ?>
	<td style="padding-top:10px;" class="nowrap" align="right" valign="top">
		<input type="checkbox" name="selection[]" value="<?= $node->id() ?>" tabindex="100" />
		<a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>" tabindex="101"><img src="<?= tpl_design_uri('image/icon/edit.gif') ?>" border="0" /></a>
	</td>
	<? } ?>
</tr>
