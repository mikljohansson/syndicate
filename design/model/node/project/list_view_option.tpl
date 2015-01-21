<? if (isset($selected) && $node->nodeId == $selected->nodeId) { ?>
	<option value="<?= $node->nodeId ?>" title="<?= tpl_attribute($node->toString()) ?>" selected="selected"><?= str_repeat('&nbsp;',$pad*4) ?><?= tpl_quote($node->toString()) ?></option>
<? } else { ?>
	<option value="<?= $node->nodeId ?>" title="<?= tpl_attribute($node->toString()) ?>"><?= str_repeat('&nbsp;',$pad*4) ?><?= tpl_quote($node->toString()) ?></option>
<? } ?>