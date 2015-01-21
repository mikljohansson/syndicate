<? 
require_once 'core/lib/rpc/RpcTransport.class.inc';
$search = Module::getInstance('search'); 

$stats = array();
$failed = array();
$backends = array();

$sections = array();
foreach ($search->getClasses() as $clsid)
	$sections[] = 'n.'.SyndNodeLib::getInheritedBranch($clsid);

foreach ($config['backends'] as $key => $urn) {
	if (null !== ($backends[$urn] = Activator::getInstance($urn)) &&
		false !== ($stat = $backends[$urn]->getStatistics($config['namespace'], $key, $sections)))
		$stats[$urn] = $stat;
	else
		$failed[] = $urn;
}

?>

<? if (!empty($failed)) { ?>
<ul class="Warning">
	<? foreach ($failed as $urn) { ?>
	<li><?= tpl_text("Host '%s' is marked as failed.", $urn) ?> <? if (count(array_filter($stats))) { ?><?= tpl_text('Load is being distributed to other servers.') ?><? } else { ?><?= tpl_text('No other servers to failover to, search engine is inoperable.') ?><? } ?></li>
	<? } ?>
</ul>
<? } ?>

<table style="width:100%;">
	<caption><?= tpl_text('Search backends') ?></caption>
	<thead>
		<tr>
			<th><?= tpl_text('URN') ?></th>
			<th><?= tpl_text('Analyzed') ?></th>
			<th><?= tpl_text('Optimized') ?></th>
			<th><?= tpl_text('Queue size') ?></th>
			<th>&nbsp;</th>
			<th>&nbsp;</th>
		</tr>
	</thead>
	<tbody>
		<? foreach (array_keys($backends) as $urn) { ?>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td><? $this->display(tpl_design_path('module/tracker/urn.tpl'),array('urn'=>$urn)) ?></td>
			<td>
				<?= tpl_date('Y-m-d H:i', $stats[$urn]['last_analyze_begin'], tpl_text('Never')) ?>
				<? if ($stats[$urn]['last_analyze_begin'] && $stats[$urn]['last_analyze_end']) { ?>
				<span class="Info">(<?= tpl_text('%d seconds', $stats[$urn]['last_analyze_end'] - $stats[$urn]['last_analyze_begin']) ?>)</span>
				<? } ?>
			</td>
			<td>
				<?= tpl_date('Y-m-d H:i', $stats[$urn]['last_optimize_begin'], tpl_text('Never')) ?>
				<? if ($stats[$urn]['last_optimize_begin'] && $stats[$urn]['last_optimize_end']) { ?>
				<span class="Info">(<?= tpl_text('%d seconds', $stats[$urn]['last_optimize_end'] - $stats[$urn]['last_optimize_begin']) ?>)</span>
				<? } ?>
			</td>
			<td><?= tpl_def($stats[$urn]['refresh_queue_size']) ?></td>
			<td>
				<? if (@$backends[$urn]->isIndexing()) { ?>
				<?= tpl_text('Indexing in progress ...') ?>
				<? } else { ?>
				<a href="<?= tpl_link_call('search','runIndexer',array('urn'=>$urn)) ?>"><?= tpl_text('Start indexer') ?></a>
				<? } ?>
			</td>
			<td><a href="<?= tpl_link_call('search','clearRefreshQueue',array('urn'=>$urn)) ?>"><?= tpl_text('Clear queue') ?></a></td>
		</tr>
		<? } ?>
	</tbody>
</table>

<table style="width:100%;">
	<caption><?= tpl_text('Indexed classes') ?></caption>
	<thead>
		<tr>
			<th>&nbsp;</th>
			<? foreach (array_keys($backends) as $urn) { ?>
			<th colspan="2"><?= tpl_chop($urn,35) ?></th>
			<? } ?>
		</tr>
		<tr>
			<th><?= tpl_text('Class') ?></th>
			<? foreach (array_keys($backends) as $urn) { ?>
			<th><?= tpl_text('Documents') ?></th>
			<th>&nbsp;</th>
			<? } ?>
		</tr>
	</thead>
	<tbody>
		<? foreach ($search->getClasses() as $clsid) { ?>
		<tr class="<?= tpl_cycle(array('odd','even')) ?>">
			<td><b><?= $clsid ?></b></td>
			<? foreach (array_keys($backends) as $urn) { ?>
			<td><?= tpl_def($stats[$urn]['sections']['n.'.SyndNodeLib::getInheritedBranch($clsid)]) ?></td>
				<? if (@$backends[$urn]->isIndexing($clsid)) { ?>
				<td><em>(<?= tpl_text('Indexing in progress ...') ?>)</em></td>
				<? } else { ?>
				<td><a href="<?= tpl_link_call('search','runClassIndexer',$clsid,array('urn'=>$urn)) ?>"><?= tpl_text('Reindex') ?></a></td>
				<? } ?>
			<? } ?>
		</tr>
		<? } ?>
	</tbody>
</table>