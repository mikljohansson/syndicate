<input type="hidden" name="mplex[relation][edit][node_id]" value="<?= $node->nodeId ?>">
<fieldset>
	<legend><?= tpl_text('Topics') ?></legend>
	<? foreach (array_keys($list) as $key) { ?>
	<div>
		<input type="hidden" name="mplex[relation][edit][all][]" value="<?= $list[$key]->nodeId ?>">
		<input style="border:0;" type="checkbox" name="mplex[relation][edit][set][]" value="<?= $list[$key]->nodeId ?>"<?= isset($selected[$key])?'checked="checked"':'' ?>>
		<span title="<?= $list[$key]->getDescription() ?>"><?= $list[$key]->toString() ?></span>
	</div>
	<? } ?>
</fieldset>