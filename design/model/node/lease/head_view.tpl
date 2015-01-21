<? $customer = $node->getCustomer(); if (!$customer->isNull()) { ?>
<a href="<?= tpl_link('user','summary',$customer->nodeId) ?>"><?= $customer->toString() ?></a>
<? } else { ?>
<a href="<?= tpl_link('inventory','view',$node->nodeId) ?>"><?= $node->toString() ?></a>
<? } ?>