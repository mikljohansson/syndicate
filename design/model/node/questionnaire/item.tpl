<div class="Item" style="margin-top:20px; margin-bottom:20px;">
	<div class="Header">
		<h3><?= $node->toString() ?></h3>
	</div>
	<div class="Abstract">
		<? $content = $node->getContent(); print $content->toString(); ?>
	</div>
	<div>
		<? $this->iterate($node->getChildren(),'item.tpl',$_data); ?>
	</div>
</div>
<hr />
