<h2><?= tpl_text('Batch create new leases') ?></h2>
<div class="indent">
	Created <?= count($leases) ?> new leases.
</div>
<br />

<?= tpl_gui_table('lease', $leases, 'view.tpl') ?>