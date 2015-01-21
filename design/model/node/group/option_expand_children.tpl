<? $this->render($node,'list_view_option.tpl') ?>
<? $this->iterate($node->getChildren(),'option_expand_children.tpl') ?>