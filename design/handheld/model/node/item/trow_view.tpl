	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<td><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>"><?= 
			tpl_def(tpl_chop($node->toString(),34)) ?></a></td>
	</tr>