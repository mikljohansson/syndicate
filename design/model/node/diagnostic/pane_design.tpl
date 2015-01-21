	<? if (count($children = $node->getChildren())) { ?>
		<? $attempt = $node->getTrainingAttempt(); ?>
		<table width="100%">
		<tbody>
			<? foreach (array_keys($children) as $key) { ?>
			<tr>
				<td><? $this->render($children[$key],'item.tpl',array('attempt'=>$attempt)) ?></td>
				<td style="padding-top:10px;" class="nowrap" align="right" valign="top">
					<input type="checkbox" name="selection[]" value="<?= $children[$key]->id() ?>" tabindex="100" />
					<a href="<?= tpl_link_call($children[$key]->getHandler(),'edit',$children[$key]->nodeId) ?>" tabindex="101"><img src="<?= tpl_design_uri('image/icon/edit.gif') ?>" border="0" /></a>
				</td>
			</tr>
			<? } ?>
		</tbody>
		</table>
	<? } ?>
	
	<ul class="Actions">
		<li><a href="<?= tpl_link_call($node->getHandler(),'insert','content',
			array('data'=>array('PARENT_NODE_ID'=>$node->nodeId))) ?>" 
			title="<?= tpl_text('Add a title, paragraph or other content to this page') 
			?>"><?= tpl_text('Add page content') ?></a></li>
		<? $this->render($node,'part_view_ole_commands.tpl') ?>
	</ul>
