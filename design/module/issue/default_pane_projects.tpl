<?
global $synd_user;
$myProjects = iterator_to_array(new MethodFilterIterator($module->getProjects($synd_user)->getIterator(), 'isPermitted', 'read'));

?>
<? if (!empty($myProjects)) { ?>
<h2><?= tpl_text('My projects') ?></h2>
<div class="indent">
	<? $this->iterate(SyndLib::sort($myProjects),'item.tpl') ?>
</div>
<? } ?>

<? 
// Root level projects
$projects = $module->getPrimaryProjects();
$projects = SyndLib::array_kdiff($projects, $myProjects);
$projects = SyndLib::filter($projects,'isPermitted','read');

if (!empty($projects)) { ?>
<h2><?= tpl_text('Projects') ?></h2>
<div class="indent">
	<? $this->iterate(SyndLib::sort($projects),'item.tpl') ?>
</div>
<? } ?>
