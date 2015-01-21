<? if (null != ($thumbUri = $node->getThumb(100))) { ?>
<a href="<?= tpl_link('user','view',$node->nodeId) ?>">
<img src="<?= $thumbUri ?>" title="<?= $node->toString() ?>" />
</a>
<? } else { ?>
<a href="<?= tpl_link('user','view',$node->nodeId) ?>"><?= $node->toString() ?></a>
<? } ?>