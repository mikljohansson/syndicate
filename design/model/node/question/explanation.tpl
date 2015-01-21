<? 
$explanation = $node->isCorrect($answer) ? $node->data['INFO_CORRECT_EXPLANATION'] : 
	$node->data['INFO_INCORRECT_EXPLANATION'];

if (null != trim($explanation)) {
	tpl_load_script(tpl_design_uri('js/layer.js'));
	?>
	<img src="<?= tpl_design_uri('image/icon/info.gif') ?>" onmouseover="synd_tooltip_show(this,'tooltip_<?= $node->nodeId ?>')" />
	<div id="tooltip_<?= $node->nodeId ?>" class="Tooltip"><?= tpl_attribute($explanation) ?></div>
	<?
} 
