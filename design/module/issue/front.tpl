<? if (!empty($newProjectIssues)) { ?>
<div class="Result">
	<? $projects = SyndLib::sort(SyndNodeLib::getInstances(array_keys($newProjectIssues))); ?>
	<? foreach (array_keys($projects) as $key) { ?>
		<a href="<?= tpl_link('issue','project',$projects[$key]->getProjectId()) ?>" title="<?= tpl_attribute($projects[$key]->getDescription()) ?>"><?= $projects[$key]->toString() ?></a>
		<span class="Info">(<a href="<?= tpl_link('issue','project',$projects[$key]->getProjectId(),'recent') ?>"><?= tpl_text('%d new issues', $newProjectIssues[$key]) ?></a>)</span><br />
	<? } ?>
</div>
<? } ?>

<? if (isset($maxprio) && count($issues = $maxprio->getInstances())) { ?>
	<h3><?= tpl_text('Priority issues') ?></h3>
	<? $this->display(tpl_design_path('gui/pager.tpl'),$maxprio->getParameters()) ?>
	<? $this->display('model/node/issue/table.tpl', array('list'=>$issues,'hideCheckbox'=>true)) ?>
<? } ?>

<? if (isset($open) && count($issues = $open->getInstances())) { ?>
	<h3><a href="<?= tpl_link('issue','issues') ?>"><?= tpl_text('Your open issues') ?></a></h3>
	<? $this->display(tpl_design_path('gui/pager.tpl'),$open->getParameters()) ?>
	<? $this->display('model/node/issue/table_client.tpl', array('list'=>$issues,'hideCheckbox'=>true)) ?>
<? } ?>

<? if (isset($closed) && count($issues = $closed->getInstances())) { ?>
	<h3><a href="<?= tpl_link('issue','issues') ?>"><?= tpl_text('Your closed issues') ?></a></h3>
	<? $this->display(tpl_design_path('gui/pager.tpl'),$closed->getParameters()) ?>
	<? $this->display('model/node/issue/table_client.tpl', array('list'=>$issues,'hideCheckbox'=>true)) ?>
<? } ?>
