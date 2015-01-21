<?
require_once 'core/lib/SyndExcel.class.inc'; 

print SyndExcel::cell();
print SyndExcel::cell(tpl_chop($node->toString(),50));
print "\r\n";

foreach ($options as $id) { 
	if (null != ($option = $node->getOption($id))) {
		print SyndExcel::cell();
		print SyndExcel::cell();
		print SyndExcel::cell($option['INFO_OPTION']);
		print "\r\n";
	}
}

?>
