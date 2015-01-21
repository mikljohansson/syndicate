<a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>" title="<?= 
	tpl_attribute(tpl_chop($node->getDescription(),60)) ?>"><?= tpl_chop($node->toString(),$maxLength) ?></a>