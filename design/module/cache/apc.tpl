<? 
if (function_exists('apc_cache_info')) { 
	$stats = apc_cache_info(); ?>
<h3><?= tpl_translate('<a href="%s"><b>%s</b></a> is loaded','http://pecl.php.net/package/apc/','APC') ?></h3>
	<div style="margin-bottom: 1em;">
		<table>
			<tr>
				<td style="width:15em;"><?= tpl_text('Uptime') ?></td>
				<td><?= tpl_duration(time()-$stats['start_time'], ' Days ') ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('Number of slots') ?></td>
				<td><?= $stats['num_slots'] ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('Memory usage') ?></td>
				<td><?= tpl_text('%.1fMb (%d cached items, %d inserts since started)',
					$stats['mem_size']/1024/1024, $stats['num_entries'], $stats['num_inserts']) ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('Cache get requests') ?></td>
				<td><?= tpl_text('%d (%d hits, %d misses, %d%% hit ratio)',
					$stats['num_hits']+$stats['num_misses'], $stats['num_hits'], $stats['num_misses'], 
					$stats['num_hits']+$stats['num_misses'] ? $stats['num_hits']/($stats['num_hits']+$stats['num_misses'])*100 : 0) ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('Number of expunges') ?></td>
				<td><?= $stats['expunges'] ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('TTL') ?></td>
				<td><?= $stats['ttl'] ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('File upload progress') ?></td>
				<td><?= $stats['file_upload_progress'] ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('Memory type') ?></td>
				<td><?= trim($stats['memory_type'],"\0") ?></td>
			</tr>
			<tr>
				<td><?= tpl_text('Locking type') ?></td>
				<td><?= trim($stats['locking_type'],"\0") ?></td>
			</tr>
		</table>
	</div>
<? } ?>
