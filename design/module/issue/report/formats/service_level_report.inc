<?
$received = $report->getReceivedIssues();
$closed = $report->getClosedIssues();
$quick = $report->getClosedIssues(60*15);
$pending = $report->getPendingIssues();

$receivedSecondLine = $report->getReceivedSecondLine();
$pendingSecondLine = $report->getPendingSecondLine();
$timeLogged = $report->getLoggedTime();

if (!$report->isSensitive()) {
	$timeStatistics = $report->getPerformanceStatistics();
	$feedback = $report->getFeedbackStatistics();
}
else {
	$timeStatistics = null;
	$feedback = null;
}

$keys = array_unique(array_merge(
	array_keys($received), 
	array_keys($closed),
	array_keys($quick),
	array_keys($pending),
	array_keys($receivedSecondLine),
	array_keys($pendingSecondLine),
	array_keys((array)$timeStatistics),
	array_keys((array)$feedback)
	));

// Rows to include
if ($report->hasInstanceGroups()) {
	$groups = SyndLib::sort(SyndNodeLib::getInstances($keys));
	if (false !== ($i = array_search('', $keys)))
		$groups[''] = SyndNodeLib::getInstance('user_case.'.tpl_text('Unassigned or unknown'));
}
else {
	rsort($keys);
	$groups = array_combine($keys, $keys);
}

