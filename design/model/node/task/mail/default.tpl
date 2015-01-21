<? $creator = $node->getCreator(); $content = $node->getContent(); ?>
** <?= $creator->toString() ?><? if (null != $creator->getPhone()) print ' ('.$creator->getPhone().')'; ?>

<?= trim($content->toString()) ?>