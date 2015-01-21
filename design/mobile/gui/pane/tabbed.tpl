<? foreach ($tabs as $tab) { 
	$name = str_replace(' ', '&nbsp;', $tab['text']);
?>
	<a href="<?= $tab['uri'] ?>"><? 
	if ($tab['selected']) { 
		$selected = $tab;
		print "<u>$name</u>";
	}
	else 
		print $name;
	?></a>&nbsp;
<? } ?>
<br />

<? 
if (!isset($selected)) 
	$selected = reset($tabs);
if (is_array($selected['template']))
	print $this->fetchnode($selected['template'][0],$selected['template'][1],$_data);
else
	$this->display($selected['template']);
?>
