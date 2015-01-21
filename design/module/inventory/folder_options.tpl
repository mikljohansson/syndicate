<? 
if (!isset($folders)) {
	$inventory = Module::getInstance('inventory');
	$folders = $inventory->getFolders();
}

$this->iterate(
	SyndLib::sort(SyndLib::filter($folders,'isPermitted','read')),
	'option_expand_children.tpl', array('selected'=>$selected));