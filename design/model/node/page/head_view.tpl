<a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>" title="<?= tpl_attribute($node->getPageNumber().' '.$node->toString()) ?>"><? 
if ($maxLength) 
	print tpl_chop($node->toString(),$maxLength);
else 
	print tpl_def($node->data['INFO_HEAD'], $node->toString()); 
?></a>