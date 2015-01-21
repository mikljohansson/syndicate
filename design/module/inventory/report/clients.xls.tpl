<?
require_once 'core/lib/SyndExcel.class.inc';
if (null == $collection)
	return null;

print SyndExcel::cell(tpl_translate('Client'));
print SyndExcel::cell(tpl_translate('Email'));

print SyndExcel::cell(tpl_translate('Costcenter'));
print SyndExcel::cell(tpl_translate('Number'));
print SyndExcel::cell(tpl_translate('Project'));

print SyndExcel::cell(tpl_translate('Model'));
print SyndExcel::cell(tpl_translate('Serial'));
print SyndExcel::cell(tpl_translate('Leased'));
print SyndExcel::cell(tpl_translate('Expires'));
print SyndExcel::cell(tpl_translate('Receipt'));

print SyndExcel::cell(tpl_translate('Address'));
print SyndExcel::cell(tpl_translate('Co'));
print SyndExcel::cell(tpl_translate('Zip code'));
print SyndExcel::cell(tpl_translate('City'));
print SyndExcel::cell(tpl_translate('Country'));

print "\n";

$contents = $collection->getFilteredContents(array('synd_node_item','synd_node_lease','synd_node_issue'));

// Preload some items
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
	$storage->getInstances($database->getCol($sql));
}

function _report_output_client($client, $costcenter, $item) {
	print SyndExcel::cell($client->toString());
	print SyndExcel::cell($client->getEmail());

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
	print SyndExcel::cell($item->data['INFO_SERIAL_MAKER']);

	if (($client instanceof synd_node_lease)) {
		print SyndExcel::cell(tpl_date('Y-m-d', $client->getCreated()));
		print SyndExcel::cell(tpl_date('Y-m-d', $client->getExpire()));
		$receipt = $client->getReceiptTemplate();		
		print SyndExcel::cell($receipt->toString());
	}
	else {
		print SyndExcel::cell('');
		print SyndExcel::cell('');
		print SyndExcel::cell('');
	}
	
	$address = $client->getAddress();
	print SyndExcel::cell($address['STREET']);
	print SyndExcel::cell($address['CO']);
	print SyndExcel::cell($address['ZIP']);
	print SyndExcel::cell($address['CITY']);
	print SyndExcel::cell($address['COUNTRY']);

	print "\r\n";
}

foreach (array_keys($contents) as $key) {
	if (($contents[$key] instanceof synd_node_item))
		_report_output_client($contents[$key]->getCustomer(), $contents[$key]->getCustomer(), $contents[$key]);
	else if (($contents[$key] instanceof synd_node_lease)) {
		foreach (array_keys($items = $contents[$key]->getItems()) as $key2) 
			_report_output_client($contents[$key], $contents[$key]->getCostcenter(), $items[$key2]);
	}
	else if (($contents[$key] instanceof synd_node_issue)) {
		$client = $contents[$key]->getCustomer();
		if (($client instanceof synd_node_lease)) {
			if (count($items = $client->getItems())) {
				foreach (array_keys($items) as $key2) 
					_report_output_client($client, $client->getCostcenter(), $items[$key2]);
			}
			else {
				_report_output_client($client, $client->getCostcenter(), SyndNodeLib::getInstance('item_case.'));
			}
		}
		else {
			_report_output_client($client, $client, SyndNodeLib::getInstance('item_case.'));
		}
	}
}
