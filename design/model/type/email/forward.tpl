

<?= tpl_translate('---------- Forwarded message ----------') ?>

<? $this->render($node,'render/text.tpl',array('message'=>$node->getMessage(),'quote'=>true)) ?>
