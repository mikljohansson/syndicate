<?
require_once 'core/lib/SyndExcel.class.inc';

print SyndExcel::cell(tpl_translate('Customer'));
print SyndExcel::cell(tpl_translate('Email'));

print SyndExcel::cell(tpl_translate('SLAs'));
print SyndExcel::cell(tpl_translate('Costcenter'));
print SyndExcel::cell(tpl_translate('Costcenter number'));
print SyndExcel::cell(tpl_translate('Project'));

print SyndExcel::cell(tpl_translate('Model'));
print SyndExcel::cell(tpl_translate('Serial'));
print SyndExcel::cell(tpl_translate('Lease Created'));
print SyndExcel::cell(tpl_translate('Lease Expires'));
print SyndExcel::cell(tpl_translate('Receipt'));

print SyndExcel::cell(tpl_translate('Address'));
print SyndExcel::cell(tpl_translate('Co'));
print SyndExcel::cell(tpl_translate('Zip code'));
print SyndExcel::cell(tpl_translate('City'));
print SyndExcel::cell(tpl_translate('Country'));

print "\r\n";

// Preload some items
$contents = $report->getContents();
$ids = SyndLib::collect($contents, 'nodeId');
$storage = SyndNodeLib::getDefaultStorage('item');
$database = $storage->getDatabase();

for ($i=0; $i<count($ids); $i+=1000) {
	$sql = "
		SELECT i.node_id FROM synd_inv_item i, synd_inv_used u
		WHERE 
			i.node_id = u.child_node_id AND
			u.ts_expire IS NULL AND
			u.parent_node_id IN (".implode(',',$database->quote(array_slice($ids, $i, 1000))).")";
	foreach ($database->getCol($sql) as $id)
		$storage->preload($id);
}

foreach ($contents as $lease) {
	$descriptions = $lease->getServiceLevelDescriptions();
	$costcenter = $lease->getCostcenter();
	
	if (!count($items = $lease->getItems()))
		$items = array(SyndNodeLib::getInstance('item_case.null'));
	
	foreach ($items as $item) {
		print SyndExcel::cell($lease->toString());
		print SyndExcel::cell($lease->getEmail());

		print SyndExcel::cell(implode(' ,', SyndLib::invoke($descriptions, 'toString')));
		if (($costcenter instanceof synd_node_costcenter)) {
			print SyndExcel::cell($costcenter->toString());
			print SyndExcel::cell($costcenter->getNumber());
			print SyndExcel::cell($costcenter->getProjectCode());
		}
		else {
			print SyndExcel::cell('');
			print SyndExcel::cell('');
			print SyndExcel::cell('');
		}

		print SyndExcel::cell($item->getModel());
		print SyndExcel::cell($item->getSerial());

		print SyndExcel::cell(tpl_date('Y-m-d', $lease->getCreated()));
		print SyndExcel::cell(tpl_date('Y-m-d', $lease->getExpire()));
		$receipt = $lease->getReceiptTemplate();		
		print SyndExcel::cell($receipt->toString());

		$address = $lease->getAddress();
		print SyndExcel::cell($address['STREET']);
		print SyndExcel::cell($address['CO']);
		print SyndExcel::cell($address['ZIP']);
		print SyndExcel::cell($address['CITY']);
		print SyndExcel::cell($address['COUNTRY']);

		print "\r\n";
	}
}
