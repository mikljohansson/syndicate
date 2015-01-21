<? 
$projects = iterator_to_array(new MethodFilterIterator($node->getArchivedProjects()->getIterator(), 'isPermitted', 'read'));
if (!empty($projects)) { ?>
<h3><?= tpl_text('Archived projects') ?></h3>
<table>
	<? foreach ($projects as $project) { ?>
	<tr>
		<td style="white-space:nowrap; padding-right:15px;">
			<a href="<?= tpl_link($project->getHandler(),'project',$project->getProjectId()) ?>"><?= $project->toString() ?></a>
		</td>
		<td><?= tpl_def($project->getDescription()) ?></td>
	</tr>
	<? } ?>
</table>
<? } ?>
