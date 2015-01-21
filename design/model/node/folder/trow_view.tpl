	<tr>
		<td>
			<a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>"><?= $node->toString() ?></a>
			<? if (null != $node->getDescription()) { ?>
			<div class="Info" style="margin-left:0.5em;"><?= $node->getDescription() ?></div>
			<? } ?>
		</td>
		<td width="10"><input type="checkbox" name="collections[]" value="<?= $node->id() ?>" /></td>
	</tr>
		