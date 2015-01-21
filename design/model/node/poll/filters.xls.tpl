<? 
if (count($node->getFilterOptions())) {
	require_once 'core/lib/SyndExcel.class.inc';
	print SyndExcel::cell(tpl_translate('%d replies matching current filters.', $node->getFilteredCount()));
	print "\r\n";

	foreach ($node->getFilterOptions() as $id => $options) {
		if (null != ($question = SyndNodeLib::getInstance($id)))
			print $this->fetchnode($question,'filter.xls.tpl',array('options' => $options));
	}
	print "\r\n";
} 
?>