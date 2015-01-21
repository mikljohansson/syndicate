<?
require_once 'core/lib/SyndExcel.class.inc';
if (null == $collection)
	return null;

if (($collection instanceof synd_type_composite_collection))
	$collections = $collection->getFragments();
else
	$collections = array($collection);

print SyndExcel::cell('Model');
foreach (array_keys($collections) as $key)
	print SyndExcel::cell($collections[$key]->toString());
if (count($collections) > 1)
	print SyndExcel::cell(tpl_translate('Sum'));
print "\r\n";

$models = array();
$counts = array();
$module = Module::getInstance('inventory');

foreach (array_keys($collections) as $key) {
	$items = $module->extractItems($collections[$key]->getFilteredContents(array('synd_node_item','synd_node_lease')));
	$counts[$key] = array();
	foreach (array_keys($items) as $key2)
		$counts[$key][$items[$key2]->getModel()]++;
	$models = array_unique(array_merge($models, array_keys($counts[$key])));
}

$i = 2;
foreach ($models as $model) {
	print SyndExcel::cell($model);
	foreach (array_keys($collections) as $key)
		print SyndExcel::cell(isset($counts[$key][$model]) ? $counts[$key][$model] : '');
	if (count($collections) > 1)
		print "=SUM(B".$i.":".(chr(ord('B')+count($collections)-1)).($i++).")\t";
	
	print "\r\n";
}

if (count(reset($counts))) {
	$i = 0;
	print SyndExcel::cell(tpl_translate('Sum'));
	for (; $i < count($collections); $i++)
		print "=SUM(".chr(ord('B')+$i)."2:".chr(ord('B')+$i).(count($models)+1).")\t";
	if (count($collections) > 1)
		print "=SUM(B2:".chr(ord('B')+($i++)-1).(count($models)+1).")\t";
	
	print "\r\n";
}

?>
