<? 
$name = $node->getName($attempt);
$oid = $option['OPTION_NODE_ID'];
$id = "{$node->nodeId}_{$option['OPTION_NODE_ID']}";
?>
<? if (null !== $answer && in_array($oid, $answer)) { ?>
	<input type="<?= $type ?>" id="<?= $id ?>" name="<?= $name ?>" value="<?= $oid ?>" checked="checked" />
<? } else { ?>	
	<input type="<?= $type ?>" id="<?= $id ?>" name="<?= $name ?>" value="<?= $oid ?>" />
<? } ?>

<? if (null !== $answer) { ?>
	<? if ($node->isCorrectOption($option['OPTION_NODE_ID']) && in_array($oid, $answer)) { ?>
		<label for="<?= $id ?>" class="Correct"><?= $label ?></label>
	<? } else if (!$node->isCorrectOption($oid) && !in_array($oid, $answer)) { ?>
		<label for="<?= $id ?>"><?= $label ?></label>
	<? } else { ?>
		<label for="<?= $id ?>" class="Incorrect"><?= $label ?></label>
	<? } ?>
<? } else { ?>
	<label for="<?= $id ?>"><?= $label ?></label>
<? } ?>