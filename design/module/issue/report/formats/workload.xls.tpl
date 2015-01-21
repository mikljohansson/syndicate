<?
require_once 'core/lib/SyndDate.class.inc';
require_once 'core/lib/SyndExcel.class.inc';

$issues = $report->getContents();
require tpl_design_path('module/issue/report/formats/workload.inc');

$row = 1;
$col = ord('A');

print SyndExcel::cell(tpl_translate('Assigned'));
print SyndExcel::cell(tpl_translate('Issue'));
print SyndExcel::cell(tpl_translate('Due'));
print SyndExcel::cell(tpl_translate('Prio'));
print SyndExcel::cell(tpl_translate('Estimate'));
print SyndExcel::cell(tpl_translate('Logged'));

print "\r\n";
$row++;

foreach ($month as $monthDate => $monthIssues) { 
	foreach ($week[$monthDate] as $weekDate => $issues) { 
		print SyndExcel::cell($weekDate);
		print SyndExcel::cell('');
		print SyndExcel::cell('');
		print SyndExcel::cell('');
		print '=SUM('.chr($col+4).($row+1).':'.chr($col+4).($row+count($issues)).")\t";
		print '=SUM('.chr($col+5).($row+1).':'.chr($col+5).($row+count($issues)).")\t";
		
		print "\r\n";
		$row++;

		foreach (array_keys($issues) as $key) {
			$assigned = $issues[$key]->getAssigned();
			print SyndExcel::cell($assigned->toString());
		
			print SyndExcel::cell($issues[$key]->getTitle());
			print SyndExcel::cell(ucwords(tpl_strftime('%a, %d %b', $issues[$key]->getResolveBy())));
		
			switch ($issues[$key]->getPriority()) { 
				case 0: print SyndExcel::cell(tpl_translate('Low')); break;
				case 1: print SyndExcel::cell(tpl_translate('Norm')); break;
				case 2: print SyndExcel::cell(tpl_translate('High')); break;
			}
			
			print SyndExcel::cell(round($issues[$key]->getEstimate()/3600,1));
			print SyndExcel::cell(round($issues[$key]->getDuration()/3600,1));

			print "\r\n";
			$row++;
		}

		print "\r\n";
		$row++;
	}
}
