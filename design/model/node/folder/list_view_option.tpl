<? if (isset($selected) && (is_object($selected) && $node->nodeId == $selected->nodeId || is_array($selected) && in_array($node->nodeId,$selected))) { ?>
	<option value="<?= $node->nodeId ?>" selected="selected"><?= str_repeat('&nbsp;',$pad*4) ?><?= $node->toString() ?></option>
<? } else { ?>
	<option value="<?= $node->nodeId ?>"><?= str_repeat('&nbsp;',$pad*4) ?><?= $node->toString() ?></option>
<? } ?>