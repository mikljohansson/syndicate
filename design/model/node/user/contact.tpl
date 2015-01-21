<a href="<?= tpl_link('user','summary',$node->nodeId) ?>"><?= $node->toString() ?></a> 
<span class="Info">(<?= $node->getLogin() ?>, <?= tpl_email($node->getEmail()) ?>)</span>