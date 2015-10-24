<? 
$uri = $node->getAttribute('svn_repository_location');
$viewvc = $node->getAttribute('svn_viewvc_location');

$revision = $node->getRevision();
if ($revision->isNull()) { ?>
<em><?= tpl_text("No revision '%s' found in repository '%s'", $node->toString(), $uri) ?>
<? } else { ?>
<div class="Revision">
	<ul class="Actions"><li><?= tpl_text('Revision %s by <em>%s</em> from <em>%s</em>', $node->toString(), $revision->getCreator(), 
		ucwords(tpl_strftime('%A, %d %b %Y %H:%M', $revision->getCreateTime()))) ?></li></ul>
	<p><?= tpl_html_format($revision->getComment()) ?></p>
	<? if (count($modifications = $revision->getModifications())) { ?>
	<table>
	<? foreach (array_keys($modifications) as $key) { ?>
		<? if ($i++ >= 15) { ?>
		<tr><td colspan="2">...</td></tr>
		<? break; } ?>
	<tr>
		<td><?= tpl_text($modifications[$key]->getTitle()) ?></td>
		<td><a href="<?= $modifications[$key]->getLocation($viewvc) ?>"><?= $modifications[$key]->getPath() ?></a></td>
	</tr>
	<? } ?>
	</table>
	<? } ?>
</div>
<? } ?>