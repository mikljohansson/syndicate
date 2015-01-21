<? if (isset($maxLength)) { ?>
<a title="<?= tpl_attribute(tpl_chop(strip_tags($node->data['INFO_BODY']),250)) 
	?>" href="<?= tpl_link($node->getHandler(),$node->objectId()) ?>"><?= 
	tpl_chop($node->data['INFO_HEAD'],$maxLength) ?></a>
<? } else { ?>
<a title="<?= tpl_attribute(tpl_chop(strip_tags($node->data['INFO_BODY']),250)) 
	?>" href="<?= tpl_link($node->getHandler(),$node->objectId()) ?>"><?= 
	tpl_chop($node->data['INFO_HEAD'],50) ?></a> (<?= $node->isClosed() ? tpl_text('Closed') : tpl_text('Open') ?>)
<? } ?>