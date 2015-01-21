<style>
	table.res_week {
		border: 2px solid #000;
		border-collapse: collapse;
		background-color: #eee;
	}

	table.res_week th {
		border: 1px solid #000;
		background-color: #eee;
		font-weight: normal;
		width: 110px;
	}

	table.res_week td {
		border: 1px solid #000;
		font-size: 8pt;
		height: 35px;
		padding: 2px;
	}

	.res_cell {
		text-align: right;
		vertical-align: bottom;
	}

	.res_event_begin {
		vertical-align: top;
	}

	.res_event_end {
		vertical-align: bottom;
	}
	
	table.res_week hr {
		border: 1px dashed #666;
		height: 1px;
		color: #666;
	}
</style>
<?
require_once 'HTML/Table.php';
require_once 'core/lib/SyndDate.class.inc';

$widget   = new HTML_Table(array('class'=>'res_week'));
$dayNames = array('Söndag', 'Måndag', 'Tisdag', 'Onsdag', 'Torsdag', 'Fredag', 'Lördag');

if (!isset($startOfDay))
	$startOfDay = 3600*6;
if (!isset($endOfDay))
	$endOfDay   = 3600*16-1;
	
$calRowOffset = 1;
$calColOffset = 1;

$calColCount = SyndDate::lengthDays($startOfWeek, $endOfWeek);
$calRowCount = SyndDate::lengthHours($startOfDay, $endOfDay);

for($i=0; $i<$calColCount; $i++) {
	$tsDay    = SyndDate::startOfDay($startOfWeek + 3600*24*$i);
	$dayName  = $dayNames[date('w', $tsDay)];
	$colIndex = $calColOffset+$i;
	
	$widget->setCellAttributes(0, $colIndex, array('class'=>'res_head'));
	$widget->setCellContents(0, $colIndex,
		'<div class="small"><b>'.$dayName.'</b><br />'.date('Y-m-d',$tsDay).'</div>','th');

	if (isset($callback_emptyCell)) {
		for ($j=0; $j < $calRowCount; $j++) {
			$cellId   = 'res_cell_'.$i.'x'.$j;
			$tsRow    = $tsDay + $startOfDay + 3600*$j + 3600;
			$content  = call_user_func_array($callback_emptyCell, 
				array_merge(array($cellId, $tsRow, $tsRow+3599),$callback_emptyCellArgs));

			$widget->updateCellAttributes($calRowOffset+$j, $colIndex, array('id'=>$cellId, 'class'=>'res_cell'));
			$widget->setCellContents($calRowOffset+$j, $colIndex, $content);
		}
	}
}

for ($i=0; $i < $calRowCount; $i++) {
	$row  = $calRowOffset+$i;
	$hour = round($startOfDay/3600)+$i+1;
	
	$widget->setCellContents($row, 0, '<div class="medium bold">'.$hour.'</div>');
}

/**
 * @access	private
 */
function _event_splitDays($startOfPeriod, $endOfPeriod, $startOfDay, $endOfDay, $startOfEvent, $endOfEvent) {
	$eventPeriods = array();

	if ($startOfEvent < $startOfPeriod)
		$tsNow = SyndDate::startOfDay($startOfPeriod);
	else
		$tsNow = SyndDate::startOfDay($startOfEvent);
	
	//debug(date('Ymd H:i',$startOfEvent), ' ', date('Ymd H:i',$endOfEvent), ' ', date('Ymd H:i',$startOfDay), ' ', date('Ymd H:i',$endOfDay));

	$i = 0;
	while ($tsNow < $endOfEvent && $tsNow < $endOfPeriod && $i++ < 100) {
		$period = array();
		
		if ($startOfEvent % (3600*24) > $endOfDay)
			continue;
		
		// If tsNow on same day as startOfEvent
		if (date('Ymd',$tsNow) == date('Ymd',$startOfEvent) && ($startOfEvent % (3600*24) + 3600 >= $startOfDay)) {
			$period['ts_start']  = $startOfEvent;
			$period['startopen'] = false;
		}
		else {
			$period['ts_start']  = $tsNow + $startOfDay + 3600;
			$period['startopen'] = true;
		}
		
		// If on same day
		if (date('Ymd',$tsNow) == date('Ymd',$endOfEvent) && ($endOfEvent % (3600*24) + 3600 <= $endOfDay)) {
			$period['ts_end']  = $endOfEvent;
			$period['endopen'] = false;
		}
		else {
			$period['ts_end']  = $tsNow + $endOfDay + 3600;
			$period['endopen'] = true;
		}
		
		//debug(date('Ymd H:i',$period['ts_start']), ' ', date('Ymd H:i',$period['ts_end']));

		$tsNow += 3600*24;
		$eventPeriods[] = $period;
	}
	
	return $eventPeriods;
}

$eventCells = array();

foreach (array_keys($list) as $key) {
	$node = $list[$key];
	
	$eventStyle = null;
	if (isset($callback_eventStyle))
		$eventStyle = call_user_func($callback_eventStyle, $node);
		
	$startOfEvent = $node->data['TS_START'];
	$endOfEvent   = $node->data['TS_STOP'];

	$eventPeriods = _event_splitDays(
		$startOfWeek, $endOfWeek, 
		$startOfDay, $endOfDay, 
		$startOfEvent, $endOfEvent);
		
	if (count($eventPeriods) > 100) {
		print 'loop '.count($eventPeriods);
		exit;
	}
	
	$colSpan = count($eventPeriods);
	
	// Start column index of event
	for ($i=0; $i<$colSpan; $i++) {
		$period   = $eventPeriods[$i];
		$colIndex = SyndDate::lengthDays($startOfWeek, $period['ts_start']) + $calColOffset - 1;

		//debug(date('Ymd H:i',$period['ts_start']), ' ', date('Ymd H:i',$period['ts_end']));

		$cellContent = null;
		$rowIndex 	 = date('H',$period['ts_start']) - $startOfDay/3600 - 1 + $calRowOffset;
		$rowSpan  	 = SyndDate::lengthHours($period['ts_start'], $period['ts_end'])+1;
		
		if (date('i', $period['ts_start']) > date('i', $period['ts_end'])) 
			$rowSpan++;

		// Search backwards in column for non spanned cell
		while ($rowIndex >= $calRowOffset) {
			if ($widget->getCellContents($rowIndex, $colIndex)) 
				break;
			$rowIndex--;
		}
		
		if (0 == $i) {
			$widget->updateCellAttributes($rowIndex, $colIndex, array('style'=>$eventStyle,'class'=>'res_event_begin'));
			$formatStart = $period['startopen'] && date('Ymd',$startOfEvent)!=date('Ymd',$period['ts_start']) ? 'Y-m-d H:i' : 'H:i';
			$formatEnd   = $period['endopen'] && date('Ymd',$endOfEvent)!=date('Ymd',$period['ts_start']) ? '<\b\r> Y-m-d H:i' : 'H:i';
			if (1 == $colSpan) 
				$cellContent .= '<b>'.date($formatStart, $startOfEvent).' -&gt; '.date($formatEnd,$endOfEvent).'</b>';
			else
				$cellContent .= '<b>'.date($formatStart, $startOfEvent).' -&gt;</b>';
			$cellContent .= $this->fetchnode($node,'week_view.tpl');
		}
		else if ($colSpan-1 == $i) {
			$widget->updateCellAttributes($rowIndex, $colIndex, array('style'=>$eventStyle,'class'=>'res_event_end'));
			$formatEnd = $period['endopen'] && date('Ymd',$endOfEvent)!=date('Ymd',$period['ts_start']) ? 'Y-m-d H:i' : 'H:i';
			$cellContent = '<b>-&gt; '.date($formatEnd,$endOfEvent).'</b>';
		}
		else { 
			$widget->updateCellAttributes($rowIndex, $colIndex, array('style'=>$eventStyle));
			$cellContent = '&nbsp;';
		}
		
		if (isset($eventCells[$rowIndex.'.'.$colIndex]))
			$cellContent = $widget->getCellContents($rowIndex, $colIndex).'<hr>'.$cellContent;
		$eventCells[$rowIndex.'.'.$colIndex] = true;

		if ($rowSpan > 1) {
			$attr = $widget->getCellAttributes($rowIndex, $colIndex);
			if (isset($attr['rowspan'])) {
				$rowSpan += $attr['rowspan'] - 1;
			}
			$widget->updateCellAttributes($rowIndex, $colIndex, array('rowspan'=>$rowSpan));
		}

		$widget->setCellContents($rowIndex, $colIndex, $cellContent);
	}
}

print $widget->toHtml();
