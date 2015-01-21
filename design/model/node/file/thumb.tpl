<? 
$file = $node->getFile(); 
$image = $node->getImage();
$title = $node->toString().' '.round($file->getSize()/1024).'Kb';
?>
<? if ($node->isPermitted('write')) { ?>
<a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>"><img src="<?= $image->getBoxedUri(175,175) ?>" class="Thumb" alt="<?= $image->toString() ?>" title="<?= $title ?>" /></a>
<? } else { ?>
<a href="<?= $file->uri() ?>"><img src="<?= $image->getBoxedUri(175,175) ?>" class="Thumb" alt="<?= $image->toString() ?>" title="<?= $title ?>" /></a>
<? } ?>