<?
header('Content-Type: application/xls');
require_once 'core/lib/SyndExcel.class.inc';
$page = $node->getPage();
$options = $node->getOptions();

print $this->fetchnode($node->getPage(),'filters.xls.tpl');

// Output question
print SyndExcel::cell($node->toString());
print "\r\n";

// Output column headers
print SyndExcel::cell();
foreach ($options as $option) 
	print SyndExcel::cell($option['INFO_OPTION']);
print "\r\n";

print SyndExcel::cell();
foreach ($options as $option) 
	print SyndExcel::cell('%');
print "\r\n\r\n";

function _percent($value) {
	return round($value * 100);
}

// Output statistics
foreach (array_keys($questions = $page->getQuestions()) as $key) {
	if (!($questions[$key] instanceof synd_node_multiple_choice))
		continue;
	
	print "\r\n";
	print SyndExcel::cell($questions[$key]->toString());
	print "\r\n";
	
	foreach ($questions[$key]->getOptions() as $nodeOption) {
		print SyndExcel::cell($nodeOption['INFO_OPTION']);
		foreach ($options as $option) {
			$column = $perColumn[$option['OPTION_NODE_ID']];
			$option = $perOption[$option['OPTION_NODE_ID']][$nodeOption['OPTION_NODE_ID']];
			print SyndExcel::cell(null != $column ? _percent($option / $column) : null);
		}
		print "\r\n";
	}
	
	print SyndExcel::cell(tpl_translate('No reply'));
	foreach ($options as $option) {
		$column = $perColumn[$option['OPTION_NODE_ID']];
		$question = $perQuestion[$option['OPTION_NODE_ID']][$questions[$key]->nodeId];
		print SyndExcel::cell(null != $column ? _percent(1 - $question / $column) : null);
	}
	print "\r\n";
}

