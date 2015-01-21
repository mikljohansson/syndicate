<? if (isset($selected) && $node->nodeId == $selected->nodeId) { ?>
	<option selected="selected" value="<?= $node->nodeId ?>"><?= $node->toString() ?></option>
<? } else { ?>
	<option value="<?= $node->nodeId ?>"><?= $node->toString() ?></option>
<? } ?>