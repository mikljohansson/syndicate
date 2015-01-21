<?
require_once 'core/lib/SyndExcel.class.inc';

print SyndExcel::cell(tpl_translate('Model'));
print SyndExcel::cell(tpl_translate('Serial'));
print "\r\n";

foreach (array_keys($contents = $report->getContents()) as $key) {
	print SyndExcel::cell($contents[$key]->getModel());
	print SyndExcel::cell($contents[$key]->data['INFO_SERIAL_MAKER']);
	print "\r\n";
}
