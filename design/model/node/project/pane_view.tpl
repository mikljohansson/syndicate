<? 
$projects = iterator_to_array(new MethodFilterIterator($node->getPublishedProjects()->getIterator(0, null, array('INFO_HEAD')), 'isPermitted', 'read'));
$archived = count($node->getArchivedProjects());

?>
<p><?= tpl_html_format($node->getDescription()) ?></p>
<ul class="Actions">
	<li><a href="<?= tpl_link($node->getHandler(),'invoke',$node->nodeId,'newIssue') ?>"><?= 
		tpl_text('New issue for %s',$node->toString()) ?></a></li>
</ul>
<? if (null != ($email = $node->getEmail())) { ?>
	<? if ($node->isIssueSender()) { ?>
	<?= tpl_translate('You can also create an issue by sending an email to %s', tpl_email($email)) ?>
	<? } else if (null != ($subjecttag = $node->getEmailSubjectTag())) { ?>
	<?= tpl_translate('You can also create an issue by sending an email to %s with %s last on the subject line', tpl_email($email, tpl_text('    %s', $subjecttag)), tpl_quote($subjecttag)) ?>
	<? } ?>
<? } ?>

<? if (!empty($projects) || !empty($archived)) { ?>
<h3><?= tpl_text('Projects') ?></h3>
<? } ?>
<? if (!empty($projects)) { ?>
<table>
	<? foreach ($projects as $project) { ?>
	<tr>
		<td style="width:15em; padding-right:15px;">
			<a href="<?= tpl_link($project->getHandler(),'project',$project->getProjectId()) ?>"><?= $project->toString() ?></a>
		</td>
		<td><?= tpl_def($project->getDescription()) ?></td>
	</tr>
	<? } ?>
</table>
<? } ?>
<? if (!empty($archived)) { ?>
<ul class="Actions">
	<li><a href="<?= tpl_view('issue','project',$node->getProjectId(),'archived') ?>"><?= tpl_text('Display %d archived projects', $archived) ?></a></li>
</ul>
<? } ?>

<? if ($node->isPermitted('write') && count($categories = $node->getLocalCategories())) { ?>
<h3><?= tpl_text('Categories') ?></h3>
<table>
	<? foreach (SyndLib::sort($categories) as $category) { ?>
	<tr>
		<td style="width:15em; padding-right:15px;">
			<a href="<?= tpl_link($node->getHandler(),'view',$category->nodeId) ?>"><?= $category->toString() ?></a>
		</td>
		<td><?= tpl_def($category->getDescription()) ?></td>
	</tr>
	<? } ?>
</table>
<? } ?>
