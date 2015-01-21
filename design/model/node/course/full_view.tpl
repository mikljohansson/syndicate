<div class="Article">
	<? $this->render($node,'part_view_header.tpl') ?>
	<? include $this->path('synd_node_page','part_view_body.tpl'); ?>
	<? $this->render($node->getPage(),'part_view_children.tpl') ?>
	<? $this->render($node->getPage(),'part_view_files.tpl') ?>

	<table style="width:100%;">
	<tr>
		<td class="top" style="padding:10px;">
			<ul class="Actions">
				<? $this->render($node->getPage(),'part_view_commands.tpl') ?>
				<li class="ProgressAction"><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId,'results') ?>" 
						title="<?= tpl_text('Show my results for this course.') 
						?>"><?= tpl_text('Results overview for %s', $node->toString()) ?></a></li>
				<? if ($node->isPermitted('admin')) { ?>
				<li><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId,'admin') ?>" 
						title="<?= tpl_text('Administer groups, clients and permissions related to this course.') 
						?>"><?= tpl_text('Administer this course') ?></a></li>
				<? } ?>
			</ul>
		</td>

		<? if ($node->isPermitted('write')) { ?>
		<td class="top" style="padding:10px;">
			<ul class="Actions">
				<? $this->render($node->getPage(),'part_view_ole_commands.tpl') ?>
			</ul>
		</td>
		<? } ?>
	</tr>
	</table>

	<? $this->render($node->getPage(),'part_view_footer.tpl') ?>
</div>