<table>
	<tr>
		<th><?= $this->text('Comment') ?></th>
		<td><?= tpl_form_textarea('mplex[mplex;'.$workflow->id().';'.$node->id().'/setComment/][comment]', $node->getComment()) ?></td>
	</tr>
	<tr>
		<th><?= $this->text('Duration') ?></th>
		<td><?= tpl_form_text('mplex[mplex;'.$workflow->id().';'.$node->id().'/setComment/][duration]', $node->getDuration()) ?></td>
	</tr>
</table>
