<table>
	<tr>
		<th><?= $this->text('Comment') ?></th>
		<td><?= $this->format($node->getComment()) ?></td>
	</tr>
	<tr>
		<th><?= $this->text('Duration') ?></th>
		<td><?= $node->getDuration() ?></td>
	</tr>
</table>
