<? 
$search = Module::getInstance('search');
$index = $search->getIndex();
?>
<div class="Help">
	<h1><?= tpl_text('Search engine users guide') ?></h1>
	<? 
	if (null != ($tpl = tpl_design_path('module/search/help/'.strtolower(get_class($index)).'.tpl',false)))
		$this->display($tpl);
	foreach (array_keys($extensions = $index->getExtensions()) as $key) { 
		if (null != ($tpl = tpl_design_path('module/search/help/'.strtolower(get_class($extensions[$key])).'.tpl',false)))
			$this->display($tpl, array('extension'=>$extensions[$key]));
	} 
	?>
</div>