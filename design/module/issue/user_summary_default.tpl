<? 
tpl_load_stylesheet(tpl_design_uri('module/issue/issue.css')); 

$limit = 15;
$offset = (int)$request['offset'];
if (null == ($order = tpl_sort_order('issue')))
	$order = array('TS_RESOLVE_BY');

?>
<h3>
	<?= tpl_text('Issues') ?>
	<span class="Info">(<?
		print tpl_text('%d open', $count = count($open));
		if ($closed)
			print ', '.tpl_translate('<a href="%s">%d closed issues</a>', tpl_link('issue','report',array('customer'=>$user->getEmail(),'subissues'=>1,'status'=>'closed','output'=>array('html'=>1))), count($closed));
		if ($assigned)
			print ', '.tpl_translate('<a href="%s">%d assigned issues</a>', tpl_link('issue','report',array('assigned'=>$user->nodeId,'subissues'=>1,'output'=>array('html'=>1))), count($assigned));
	?>)</span>
</h3>
<ul class="Actions">
	<li><a href="<?= tpl_link('issue','newIssue',array('customer'=>$user->nodeId)) ?>" accesskey="n" title="<?= tpl_text('Accesskey: %s','N') ?>"><?= tpl_text('New issue from %s', $user->toString()) ?></a></li>
</ul>

<? if (count($projects = iterator_to_array(new MethodFilterIterator($projects->getIterator(), 'isPermitted', 'read')))) { ?>
<p>
	<strong><?= tpl_text('Projects:') ?></strong>
	<? foreach ($projects as $project) { ?>
		<? if ($i++) print ', '; ?>
		<? $this->render($project,'head_view.tpl') ?>
	<? } ?>
</p>
<? } ?>

<? if ($count) { ?>
<? $this->display(tpl_design_path('gui/pager.tpl'),array('offset'=>$offset,'limit'=>$limit,'count'=>$count)) ?>
<? $this->display('model/node/issue/table.tpl',array('list'=>$open->getIterator($offset,$limit,$order))) ?>
<? } ?>
