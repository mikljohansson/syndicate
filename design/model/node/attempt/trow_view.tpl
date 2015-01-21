	<tr>
		<td style="width:85px;"><? $this->render($node,'head_view_progress.tpl') ?></td>
		<td style="padding-right:10px;"><? $this->render($node->getParent(),'head_view.tpl') ?></td>
		<td style="padding-right:10px;">
			<a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>">
			<?= ucwords(tpl_strftime('%A, %d %B %Y %H:%M', $node->data['TS_CREATE'])) ?></a>
		</td>
		<td><?= $node->getCorrectCount() ?>/<?= $node->getAnswerCount() ?> </td>
		<? if ($node->isPermitted('write')) { ?>
		<td><input type="checkbox" name="selection[]" value="<?= $node->id() ?>" /></td>
		<? } ?>
	</tr>