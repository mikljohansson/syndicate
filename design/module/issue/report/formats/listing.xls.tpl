<?
require_once 'core/lib/SyndExcel.class.inc';

print SyndExcel::cell(tpl_translate('Customer'));
print SyndExcel::cell(tpl_translate('Assigned'));
print SyndExcel::cell(tpl_translate('Title'));
print SyndExcel::cell(tpl_translate('Id'));
print SyndExcel::cell(tpl_translate('Project'));

print SyndExcel::cell(tpl_translate('Reported'));
print SyndExcel::cell(tpl_translate('Updated'));
print SyndExcel::cell(tpl_translate('Due'));
print SyndExcel::cell(tpl_translate('Closed'));

print SyndExcel::cell(tpl_translate('Time spent'));
print SyndExcel::cell(tpl_translate('Time estimate'));
print "\r\n";

foreach (array_keys($contents = $report->getContents()) as $key) {
	print SyndExcel::cell($contents[$key]->getCustomer()->toString());
	print SyndExcel::cell($contents[$key]->getAssigned()->toString());
	print SyndExcel::cell($contents[$key]->getTitle());
	print SyndExcel::cell($contents[$key]->objectId());
	print SyndExcel::cell($contents[$key]->getParent()->toString());

	print SyndExcel::cell(tpl_strftime('%Y-%m-%d', (string)$contents[$key]->data['TS_CREATE']));
	print SyndExcel::cell(tpl_strftime('%Y-%m-%d', (string)$contents[$key]->data['TS_UPDATE']));
	print SyndExcel::cell(tpl_strftime('%Y-%m-%d', (string)$contents[$key]->data['TS_RESOLVE_BY']));
	print SyndExcel::cell(tpl_strftime('%Y-%m-%d', (string)$contents[$key]->data['TS_RESOLVE']));

	print SyndExcel::cell(round($contents[$key]->getDuration()/3600,1));
	print SyndExcel::cell(round($contents[$key]->getEstimate()/3600,1));
	print "\r\n";
}
