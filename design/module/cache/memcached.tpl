<? 
global $synd_config;
$cache = Module::getInstance('cache');
if (($cache->_strategy instanceof MemcachedStrategy)) { 
	$servers = $cache->_strategy->getConnection()->getExtendedStats();
?>
<h3><?= tpl_translate('<a href="%s"><b>%s</b></a> is loaded','http://www.danga.com/memcached/','memcached') ?></h3>
	<? foreach ($servers as $host => $stats) { ?>
	<div style="margin-bottom: 1em;">
		<table>
			<tr>
				<th style="width:15em;"><?= tpl_text('Host') ?></th>
				<th><?= $host ?> (pid <?= $stats['pid'] ?>)</th>
			</tr>
			<? if (false === $stats) { ?>
			<tr>
				<td colspan="2">
					<div class="Warning">
						<?= tpl_text('Host is marked as failed.') ?>
						<? if (count(array_filter($servers))) { ?>
							<?= tpl_text('Load is being distributed to other servers') ?>
						<? } else { ?>
							<?= tpl_text('No other servers to failover to, memcache is inoperable') ?>
						<? } ?>
					</div>
				</td>
			</tr>
			<? } else { ?>
			<tr>
				<td><?= tpl_text('Uptime') ?></td>
				<td><?= tpl_duration($stats['uptime'], ' Days ') ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('Resource usage') ?></td>
				<td><?= tpl_text('%s user (%s system)',
					tpl_duration(round(substr($stats['rusage_user'],0,strpos($stats['rusage_user'],':')))),
					tpl_duration(round(substr($stats['rusage_system'],0,strpos($stats['rusage_system'],':'))))) ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('Active connections') ?></td>
				<td><?= tpl_text('%d (total of %d since started)', 
					$stats['curr_connections'], $stats['total_connections']) ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('Memory limit') ?></td>
				<td><?= tpl_text('%dMb', $stats['limit_maxbytes']/1024/1024) ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('Memory usage') ?></td>
				<td><?= tpl_text('%.1fMb (%d cached items, total of %d since started)',
					$stats['bytes']/1024/1024, $stats['curr_items'], $stats['total_items']) ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('Cache get requests') ?></td>
				<td><?= tpl_text('%d (%d hits, %d misses, %d%% hit ratio)',
					$stats['cmd_get'], $stats['get_hits'], $stats['get_misses'], $stats['cmd_get'] ? $stats['get_hits']/$stats['cmd_get']*100 : 0) ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('Cache set requests') ?></td>
				<td><?= $stats['cmd_set'] ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('Traffic from frontends') ?></td>
				<td><?= tpl_text('%.1fMb', $stats['bytes_read']/1024/1024) ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('Traffic to frontends') ?></td>
				<td><?= tpl_text('%.1fMb', $stats['bytes_written']/1024/1024) ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('Version') ?></td>
				<td><?= $stats['version'] ?></td>
			</tr>
			<? } ?>
		</table>
	</div>
	<? } ?>
<? } ?>
