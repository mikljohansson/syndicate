<table>
<tr>
	<td valign="top">
		<a href="<?= $node->uri() ?>" title="<?= basename($node->path()) ?>" >
		<img src="<?= $node->getBoxedUri(60,60) ?>" border="0" />
		</a>
	</td>
	<td valign="top">
		<a href="<?= $node->uri() ?>"><?= basename($node->path()) ?></a><br />
		(<?= $node->getWidth() ?>x<?= $node->getHeight() ?>px, <?= round($node->getSize()/1000,1) ?>Kb)
	</td>
</tr>
</table>