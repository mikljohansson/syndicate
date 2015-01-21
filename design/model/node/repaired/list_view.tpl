<? $item = $node->getItem(); ?>
<? $this->render($item,'head_view.tpl') ?> <span class="Info">(<? $this->render($item->getParent(),'head_view.tpl') ?>)</span><br />