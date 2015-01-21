<? $name = str_repeat('&nbsp;',(count($node->getBranch())-1)*4).$node->toString(); ?>
<? if (isset($selected) && $node->nodeId == $selected->nodeId) { ?>
	<option selected="selected" value="<?= $node->nodeId ?>"><?= $name ?></option>
<? } else { ?>
	<option value="<?= $node->nodeId ?>"><?= $name ?></option>
<? } ?>