<? if (isset($node->data['INFO_PHOTO'])) { ?>
<a href="<?= tpl_link('user','view',$node->nodeId) ?>">
<img src="<?= $node->data['INFO_PHOTO']->getResizedUri(40,40) ?>" title="<?= $node->toString() ?>" />
</a>
<? } ?>