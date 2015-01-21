<? 
$sel = false;
if (isset($selected)) {
	if (!is_array($selected)) {
		$sel = $node->nodeId == $selected->nodeId;
	}
	else if (isset($selected[$node->nodeId])) {
		$sel = true;
	}
}

?>
<option class="Keyword" value="<?= $node->nodeId ?>" title="<?= tpl_attribute($node->toString()) ?>" <?= $sel ? 'selected="selected"' : '' ?> <?= $disabled ? 'disabled="disabled"' : '' ?>><?= str_repeat('&nbsp;',$pad*4) ?><?= tpl_quote($node->toString()) ?></option>
