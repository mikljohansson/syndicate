<ul class="Flow">
<? 
foreach ($tabs as $tab) {
	if (!isset($selected)) { 
		?><li class="CompletedStep"><a href="<?= $tab['uri'] ?>"><?= $tab['text'] ?></a></li><? 
	} else { 
		?><li><?= $tab['text'] ?></li><? 
	}

	if ($tab['selected']) 
		$selected = $tab;
} 

?>
</ul>
<? 

if (!isset($selected)) 
	$selected = reset($tabs);
if (is_array($selected['template']))
	print $this->fetchnode($selected['template'][0],$selected['template'][1],$_data);
else
	$this->display($selected['template']);
