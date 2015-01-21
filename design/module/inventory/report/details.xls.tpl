<?
require_once 'core/lib/SyndExcel.class.inc';
require_once 'core/lib/SyndDate.class.inc';
if (null == $collection)
	return null;

$section = array('synd_node_item','synd_node_lease','synd_node_repair');
$module = Module::getInstance('inventory');
$items = $module->extractItems($collection->getFilteredContents($section));

$fields = array();
$seen = array();

foreach (array_keys($items) as $key) {
	$class = $items[$key]->getClass();
	if (!$class->isNull() && !in_array($class->nodeId, $seen)) {
		$seen[] = $class->nodeId;
		$fields = array_merge($fields, SyndLib::invoke($class->getFields(),'toString'));
	}
}

print SyndExcel::cell(tpl_translate('Category'));
print SyndExcel::cell(tpl_translate('Type'));
print SyndExcel::cell(tpl_translate('S/N'));
print SyndExcel::cell(tpl_translate('Maker S/N'));
print SyndExcel::cell(tpl_translate('Installation ID'));

print SyndExcel::cell(tpl_translate('Client'));
print SyndExcel::cell(tpl_translate('Client Login'));
print SyndExcel::cell(tpl_translate('Client SSN'));
print SyndExcel::cell(tpl_translate('Location'));

print SyndExcel::cell(tpl_translate('Purchase date'));
print SyndExcel::cell(tpl_translate('Purchase value'));
print SyndExcel::cell(tpl_translate('Running cost'));

$tsDeprStart1 = SyndDate::startOfYear(time());
$tsDeprStop1 = SyndDate::endOfYear(time());

$tsDeprStart2 = SyndDate::startOfYear(strtotime('+1 year'));
$tsDeprStop2 = null;

print SyndExcel::cell(tpl_translate('Depreciation %d', date('Y', $tsDeprStart1)));
print SyndExcel::cell(tpl_translate('Depreciation %s', date('Y-m-d', $tsDeprStart2)));

foreach ($fields as $name)
	print SyndExcel::cell($name);

print "\n";

foreach (array_keys($items) as $key) {
	$class = $items[$key]->getClass();
	print SyndExcel::cell($class->toString());
	print SyndExcel::cell(tpl_def($items[$key]->getTitle(),tpl_translate('Unknown')));
	print SyndExcel::cell($items[$key]->data['INFO_SERIAL_INTERNAL']);
	print SyndExcel::cell($items[$key]->data['INFO_SERIAL_MAKER']);

	$installation = $items[$key]->getInstallation();
	print SyndExcel::cell($installation->isNull() ? null : $installation->getInstallationNumber());

	$client = $items[$key]->getCustomer();
	print SyndExcel::cell($client->toString());
	print SyndExcel::cell($client->getLogin());
	print SyndExcel::cell($client->getSocialSecurityNumber());
	print SyndExcel::cell($items[$key]->data['INFO_LOCATION']);

	print SyndExcel::cell(tpl_date('Y-m-d', $items[$key]->data['TS_DELIVERY']));
	print SyndExcel::cell($items[$key]->getPurchaseValue());
	print SyndExcel::cell($items[$key]->getRunningCost());

	print SyndExcel::cell($items[$key]->getDepreciationCost($tsDeprStart1, $tsDeprStop1));
	print SyndExcel::cell($items[$key]->getDepreciationCost($tsDeprStart2));
	
	$values = $items[$key]->getValues();
	foreach (array_keys($fields) as $key)
		print SyndExcel::cell($values[$key]);
	
	print "\n";
}

