<a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>" title="<?= 
	tpl_text(tpl_value($node->data['INFO_DESC'])) ?>"><?= $node->toString() ?></a>