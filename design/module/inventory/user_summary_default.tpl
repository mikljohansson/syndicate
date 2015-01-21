<h3><?= tpl_text('Service level agreements') ?></h3>
<? if (!empty($descriptions)) { ?>
<? foreach (array_keys(SyndLib::sort($descriptions)) as $key) {
	if ($i++) print ', ';
	print $this->fetchnode($descriptions[$key],'head_view.tpl');
} ?>
<? } else { ?>
<em><?= tpl_text('No service level agreements found') ?></em>
<? } ?>

<h3>
	<? if ($prevLeasesCount) { ?>
	<?= tpl_translate('Leases <span class="Info">(%d active, <a href="%s">%d terminated leases</a>)</span>', count($leases), 
		tpl_link('inventory','leases',array('customer'=>$user->getEmail(),'empty'=>1,'output'=>array('html'=>1))), $prevLeasesCount) ?>
	<? } else { ?>
	<?= tpl_text('Leases <span class="Info">(%d active)</span>', count($leases)) ?>
	<? } ?>
</h3>
<ul class="Actions">
	<li><a href="<?= tpl_link_call('inventory','newLease',$user->nodeId) ?>"><?= tpl_text('New lease for %s', $user->toString()) ?></a></li>
</ul>
<? if (!empty($leases)) { ?>
<div class="Enumeration">
	<? $this->iterate($leases,'item.tpl') ?>
</div>
<? } ?>

<? if (!empty($items)) { ?>
<h3><?= tpl_text('Non leased items') ?></h3>
<?= tpl_gui_table('item',$items,'view.tpl',array('hideCheckbox'=>true)) ?>
<? } ?>
