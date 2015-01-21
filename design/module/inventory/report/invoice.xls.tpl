<?
require_once 'core/lib/SyndExcel.class.inc';
require_once 'core/lib/SyndDate.class.inc';
if (null == $collection)
	return null;

$section = array('synd_node_item','synd_node_lease','synd_node_repair');
$module = Module::getInstance('inventory');

function _callback_compare(&$a, &$b) {
	$aOwner = $a->getOwner();
	$bOwner = $b->getOwner();
	if (0 != ($v = strcasecmp($aOwner->toString(), $bOwner->toString())))
		return $v;

	$aClient = $a->getCustomer();
	$bClient = $b->getCustomer();
	if (0 != ($v = strcasecmp($aClient->toString(), $bClient->toString())))
		return $v;
		
	$aClass = $a->getClass();
	$bClass = $b->getClass();
	return strcasecmp($aClass->toString(), $bClass->toString());
}

$items = $module->extractItems($collection->getFilteredContents($section));
uasort($items, '_callback_compare');

print SyndExcel::cell(tpl_translate('Client/Owner'));
print SyndExcel::cell(tpl_translate('Category'));
print SyndExcel::cell(tpl_translate('Type'));
print SyndExcel::cell(tpl_translate('S/N'));
print SyndExcel::cell(tpl_translate('Maker S/N'));
print SyndExcel::cell(tpl_translate('Installation ID'));

print SyndExcel::cell(tpl_translate('Purchase date'));
print SyndExcel::cell(tpl_translate('Purchase value'));
print SyndExcel::cell(tpl_translate('Running cost'));

$tsDeprStart1 = SyndDate::startOfYear(time());
$tsDeprStop1 = SyndDate::endOfYear(time());

$tsDeprStart2 = SyndDate::startOfYear(strtotime('+1 year'));
$tsDeprStop2 = SyndDate::endOfYear(strtotime('+1 year'));

print SyndExcel::cell(tpl_translate('Depreciation %d', date('Y', $tsDeprStart1)));
print SyndExcel::cell(tpl_translate('Depreciation %d', date('Y', $tsDeprStart2)));

print SyndExcel::cell(tpl_translate('Location'));
print SyndExcel::cell(tpl_translate('Client login'));
print SyndExcel::cell(tpl_translate('Client id'));
print SyndExcel::cell(tpl_translate('Social security number'));

print SyndExcel::cell(tpl_translate('Phone'));
print SyndExcel::cell(tpl_translate('Address'));
print SyndExcel::cell(tpl_translate('Co'));
print SyndExcel::cell(tpl_translate('Zip code'));
print SyndExcel::cell(tpl_translate('City'));
print SyndExcel::cell(tpl_translate('Country'));

print "\n";

$prevOwner = null;
$from = 3;
$to = 3;

$rows = array();

foreach (array_keys($items) as $key) {
	$client = $items[$key]->getCustomer();
	$owner = $items[$key]->getOwner();

	if (null === $prevOwner || $prevOwner->nodeId != $owner->nodeId) {
		if (null !== $prevOwner) {
			$rows[] = _report_sum($from, $to);
			print "\n";
			$to++;
		}

		print SyndExcel::cell($owner->toString());
		print "\n";
		$to++;
		
		$prevOwner = $owner;
	}
	
	print SyndExcel::cell($client->toString());

	$class = $items[$key]->getClass();
	print SyndExcel::cell($class->toString());
	print SyndExcel::cell(tpl_def($items[$key]->getTitle(),tpl_translate('Unknown')));
	print SyndExcel::cell($items[$key]->data['INFO_SERIAL_INTERNAL']);
	print SyndExcel::cell($items[$key]->data['INFO_SERIAL_MAKER']);
	
	$installation = $items[$key]->getInstallation();
	print SyndExcel::cell($installation->isNull() ? null : $installation->getInstallationNumber());

	print SyndExcel::cell(tpl_date('Y-m-d', $items[$key]->data['TS_DELIVERY']));
	print SyndExcel::cell($items[$key]->getPurchaseValue());
	print SyndExcel::cell($items[$key]->getRunningCost());

	print SyndExcel::cell($items[$key]->getDepreciationCost($tsDeprStart1, $tsDeprStop1));
	print SyndExcel::cell($items[$key]->getDepreciationCost($tsDeprStart2, $tsDeprStop2));
	
	print SyndExcel::cell($items[$key]->data['INFO_LOCATION']);
	print SyndExcel::cell($client->getLogin());
	print SyndExcel::cell($client->objectId());
	print SyndExcel::cell($client->getSocialSecurityNumber());
	print SyndExcel::cell($client->getPhone());
	
	$address = $client->getAddress();
	print SyndExcel::cell($address['STREET']);
	print SyndExcel::cell($address['CO']);
	print SyndExcel::cell($address['ZIP']);
	print SyndExcel::cell($address['CITY']);
	print SyndExcel::cell($address['COUNTRY']);
	
	print "\n";
	$to++;
}

$rows[] = _report_sum($from, $to);

function _report_sum(&$from, &$to) {
	$end = $to-2;
	print SyndExcel::cell(tpl_translate('Sum'));
	print "\t\t\t\t\t\t";
	print "=SUM(H$from:H$end)\t";
	print "=SUM(I$from:I$end)\t";
	print "=SUM(J$from:J$end)\t";
	print "=SUM(K$from:K$end)\t";
	print "\n";
	
	$result = array(
		"H$from:H$end", 
		"I$from:I$end",
		"J$from:J$end",
		"K$from:K$end",
		);
	
	$to += 1;
	$from = $to;
	
	return $result;
}

print SyndExcel::cell(tpl_translate('Totals'));
print "\t\t\t\t\t\t";
print "=SUM(".implode(';', SyndLib::array_collect($rows,0)).")\t";
print "=SUM(".implode(';', SyndLib::array_collect($rows,1)).")\t";
print "=SUM(".implode(';', SyndLib::array_collect($rows,2)).")\t";
print "=SUM(".implode(';', SyndLib::array_collect($rows,3)).")\t";
print "\n\n";
