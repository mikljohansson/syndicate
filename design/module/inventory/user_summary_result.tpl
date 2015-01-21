<? 
foreach (array_keys(SyndLib::sort($descriptions)) as $key) { 
	if ($i++) print ', ';
	print $this->fetchnode($descriptions[$key],'head_view.tpl');
}
