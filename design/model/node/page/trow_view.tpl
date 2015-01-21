	<? global $synd_user; ?>
	<tr>
		<td width="25"><? $this->render($node,'icon.tpl',$_data) ?></td>
		<td class="Title"><? $this->render($node,'head_view.tpl',$_data) ?></td>
		<td><?= $node->getDescription() ?></td>
		<td width="50">&nbsp;</td>
		<td width="70"><? $this->render($node->getBestProgressAttempt($synd_user),'head_view_progress.tpl') ?></td>
		<? if ($node->isPermitted('write')) { ?>
		<td width="25"><input type="checkbox" name="selection[]" value="<?= $node->id() ?>" /></td>
		<? } ?>
	</tr>