<div>
	<? $this->render($node,'part_view_header.tpl',$_data) ?>
	<? $this->render($node,'part_view_body.tpl',$_data) ?>
	<? $this->render($node,'part_view_children.tpl') ?>
	<? $this->render($node,'part_view_files.tpl') ?>
	
	<table>
	<tr>
		<td class="top" style="padding:10px;">
			<ul class="Actions">
				<? $this->render($node,'part_view_commands.tpl') ?>
			</ul>
		</td>

		<? if ($node->isPermitted('write')) { ?>
		<td class="top" style="padding:10px;">
			<ul class="Actions">
				<? $this->render($node,'part_view_ole_commands.tpl') ?>
			</ul>
		</td>
		<? } ?>
	</tr>
	</table>

	<? $this->render($node,'part_view_footer.tpl') ?>
</div>