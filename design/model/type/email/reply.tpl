


<?= tpl_translate('%s wrote:', $node->getSenderName()) ?>

<? $this->render($node,'render/text.tpl',array('message'=>$node->getMessage(),'quote'=>true,'headers'=>false)) ?>
