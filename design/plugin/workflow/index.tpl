<h1><?= tpl_text('Workflows') ?></h1>
<? 
global $synd_user;
$module = Module::getInstance('issue');
$projects = $module->getProjectTree(new MethodDecider('hasPermission', array($synd_user, 'manage')), $synd_user);

$workflows = synd_node_workflow::getEntityQuery(SyndNodeLib::getDefaultStorage('workflow'));
$w = $workflows->join('synd_workflow');
$workflows->in("$w.parent_node_id", new MemberIterator($projects, 'nodeId'));

if (!count($order = tpl_sort_order('workflow',"$w.")))
	$order = array("$w.INFO_HEAD");

if (($count = count($workflows))) { 
	$offset = (int)$request['offset'];
	$limit = 75; ?>
	<div class="Result">
		<?= tpl_text('Displaying %d-%d of %d workflows', $offset+1, $offset+min($count-$offset,$limit), $count) ?>
		<? $this->display(tpl_design_path('gui/pager.tpl'),array('offset'=>$offset,'limit'=>$limit,'count'=>$count,'offset_variable_name'=>'offset')) ?>
	</div>
	<? $this->display('model/node/workflow/table.tpl',array('list'=>$workflows->getEntities()->getIterator($offset,$limit,$order))) ?>
<? } else { ?>
	<div class="Result">
		<?= tpl_text("No workflows were found") ?>
	</div>
<? } ?>
