<?
$parts = explode(';', $urn, 2);
$formatter = parse_url($parts[0]);
$endpoint = parse_url($parts[1]);

?>
<span title="<?= tpl_attribute($urn) ?>">
	<?= isset($formatter['scheme']) ? $formatter['scheme'] : $formatter['path'] ?>;<?= 
		isset($endpoint['scheme'])?"{$endpoint['scheme']}://":'' ?><?= tpl_chop($endpoint['host'],15) ?><?= 
		isset($endpoint['port'])?":{$endpoint['port']}":'' ?><?= 
		isset($endpoint['path'])?tpl_chop($endpoint['path'],15):'' ?>
</span>