<?php
$week = array();
$month = array();

foreach (array_keys($issues) as $key) {
	$monthDate = ucwords(tpl_strftime('%B, %Y',$issues[$key]->getResolveBy()));
	
	$weekDate = SyndLib::text('Week %s', date('W', $issues[$key]->getResolveBy()));
	$weekDate .= ' ('.ucwords(strftime('%d',SyndDate::startOfWeek($issues[$key]->getResolveBy()))).' - '.
					  ucwords(strftime('%d %B %Y',SyndDate::endOfWeek($issues[$key]->getResolveBy()))).')';
	
	$month[$monthDate][] = $issues[$key];
	$week[$monthDate][$weekDate][] = $issues[$key];
}

$prioLevels = array(
	0 => SyndLib::text('Low'),
	1 => SyndLib::text('Norm'),
	2 => SyndLib::text('High'));
