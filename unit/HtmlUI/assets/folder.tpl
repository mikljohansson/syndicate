<? 
$items = array();
for ($iterator->rewind(); $iterator->valid(); $iterator->next()) {
	if ('.' != substr($iterator->getFilename(),0,1) && '_' != substr($iterator->getFilename(),0,1))
		$items[$iterator->getFilename()] = array($iterator->current(), $iterator->getChildren());
}
if (empty($items))
	return;

$query = '';
foreach ($options as $key => $value)
	$query .= '&amp;'.$key.'='.rawurlencode($value);

?>
<ul>
	<? 

	uksort($items, 'strnatcasecmp');
	foreach ($items as $item) { 
		?><li><a href="?sq=<?= rawurlencode(substr($item[0]->getPathname(),strlen($root)+1)) ?><?= $query ?>"><?= $item[0]->getFilename() ?></a></li><?
		if (0 === strpos($expand, $item[0]->getPathname()))
			$this->display('assets/folder.tpl', array('root'=>$root,'options'=>$options,'iterator'=>$item[1],'expand'=>$expand));
	} 
	
	?>
</ul>