<? foreach ($interfaces as $mac => $vlan) { ?>
address <?= preg_replace('/([a-f\d]{2}):([a-f\d]{2}):([a-f\d]{2}):([a-f\d]{2}):([a-f\d]{2}):([a-f\d]{2})/iS','\1\2.\3\4.\5\6', $mac) ?> vlan-name <?= $vlan ?>

<? } ?>