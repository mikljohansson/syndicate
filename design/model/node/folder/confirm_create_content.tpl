<div class="Article">
	<h1><?= tpl_text('Create content') ?></h1>
	<p><?= tpl_text('Choose the appropriate item from the list') ?></p>
	
	<dl class="Actions">
		<? foreach ($node->getClasses() as $class) { ?>
		<dt><a href="<?= tpl_link_jump($node->getHandler(),'invoke',$node->nodeId,'newContent',array('class'=>$class)) ?>"><? include $this->path("synd_node_$class",'class_title.tpl'); ?></a></dt>
		<dd><? include $this->path("synd_node_$class",'class_description.tpl'); ?></dd>
		<? } ?>
	</dl>
</div>