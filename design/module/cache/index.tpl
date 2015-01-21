<? global $synd_config; ?>

<? $this->display(tpl_design_path('module/cache/memcached.tpl')) ?>
<? $this->display(tpl_design_path('module/cache/apc.tpl')) ?>
<? $this->display(tpl_design_path('module/cache/xcache.tpl')) ?>

<h3><?= tpl_text('Filesystem cache') ?></h3>
<div class="indent">
	<?= tpl_text('Cache directory: %s', $synd_config['dirs']['cache'].'fs/') ?><br />
	<?= tpl_text('Disk usage: %.1fMb', SyndLib::file_total_size($synd_config['dirs']['cache'].'fs/')/1024/1024) ?>
</div>
