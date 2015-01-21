<h3><?= tpl_text('Clients with many repairs') ?></h3>
<?
global $synd_maindb;
$sql = "
	SELECT i.CLIENT_NODE_ID, count(*) CNT FROM synd_issue i 
	WHERE 
		i.client_node_id IS NOT NULL AND
		(i.node_id LIKE 'repair.%' OR i.node_id LIKE 'replace.%')
	HAVING count(*) >= 2
	GROUP BY i.client_node_id
	ORDER BY cnt DESC";
$rows = $synd_maindb->getAll($sql);
SyndNodeLib::getInstances(SyndLib::array_collect($rows, 'CLIENT_NODE_ID'));

?>

<div class="indent">
	<? if (count($rows)) { ?>
		<table>
			<tr>
				<th><?= tpl_text('Client') ?></th>
				<th><?= tpl_text('Number of repairs') ?></th>
			</tr>
			<? foreach ($rows as $row) { ?>
			<tr class="<?= tpl_cycle(array('odd','even')) ?>">
				<td style="width:300px;">
					<? 
					if ((($client = SyndNodeLib::getInstance($row['CLIENT_NODE_ID'])) instanceof synd_node_lease))
						$client = $client->getCustomer(); 
					?>
					<a href="<?= tpl_link('user','summary',$client->nodeId) ?>"><?= $client->toString() ?></a>
				</td>
				<td><?= $row['CNT'] ?></td>
			</tr>
			<? } ?>
		</table>
	<? } else { ?>
		<em><?= tpl_text('No clients with many repairs found') ?></em>
	<? } ?>
</div>