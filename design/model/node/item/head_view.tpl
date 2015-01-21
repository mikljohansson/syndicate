<a title="<?= $node->toString() ?>" href="<?= tpl_link('inventory','view',$node->nodeId) ?>"><? 
if ($node->data['INFO_SERIAL_INTERNAL'] || $node->data['INFO_SERIAL_MAKER']) { ?><?= 
	tpl_def($node->data['INFO_SERIAL_MAKER'], $node->data['INFO_SERIAL_INTERNAL']) 
	?>: <? } ?><?= $node->toString() ?></a>
