<?php
require_once 'service_level_report.inc';

$builder->th('');
$builder->th(tpl_text('Issues'), null, null, tpl_text('Total number of issues matching the current filters'));
$builder->th(tpl_text('2nd line'), null, null, tpl_text('Number of issues where 2nd line subissues exist'));
$builder->th(tpl_text('Closed'), null, null, tpl_text('Number of closed issues'));
$builder->th(tpl_text('Closed within 15 min'), null, null, tpl_text('Issues closed within 15 minutes of being created'));
$builder->th(tpl_text('Open'), null, null, tpl_text('Number of open issues'));
$builder->th(tpl_text('Open (2nd line)'), null, null, tpl_text('Number of open issues waiting for 2nd line issues to be closed'));
$builder->th(tpl_text('Time logged'), null, null, tpl_text('Total number of hours logged'));
$builder->th(tpl_text('Minutes per issue'), null, null, tpl_text('Average number of minutes logged per issue'));

if (!$report->isSensitive()) {
	$builder->th(tpl_text('Time to solution'), null, null, tpl_text('Number of days from when the issue is reported until it has been closed, excluding issues with time greater than 30 days'));
	$builder->th(tpl_text('Time to start'), null, null, tpl_text('Number of days from when the issue is flagged as active until it has been closed'));
	$builder->th(tpl_text('Accuracy'), null, null, tpl_text('Average accuracy in days against the resolve by date (negative values indicate overdue issues)'));
	$builder->th(tpl_text('Feedback (%)'), null, null, tpl_text('Average client satisfaction in percent'));
	$builder->th(tpl_text('Feedback replies'), null, null, tpl_text('Number of feedback replies'));
}

$builder->tr();

foreach (array_keys($groups) as $key) {
	$builder->td(is_object($groups[$key]) ? $groups[$key]->toString() : $groups[$key]);
	$builder->td(isset($received[$key]) ? $received[$key] : null, null,
		$selector ? tpl_uri_merge(array($selector=>$groups[$key]->nodeId,'report'=>'list')) : null);
	$builder->td(isset($receivedSecondLine[$key]) ? $receivedSecondLine[$key] : null);
	$builder->td(isset($closed[$key]) ? $closed[$key] : null, null,
		$selector ? tpl_uri_merge(array($selector=>$groups[$key]->nodeId,'report'=>'list','status'=>'closed')) : null);
	$builder->td(isset($quick[$key]) ? $quick[$key] : null);
	$builder->td(isset($pending[$key]) ? $pending[$key] : null, null,
		$selector ? tpl_uri_merge(array($selector=>$groups[$key]->nodeId,'report'=>'list','status'=>'open')) : null);
	$builder->td(isset($pendingSecondLine[$key]) ? $pendingSecondLine[$key] : null);
	$builder->td(isset($timeLogged[$key]) ? round($timeLogged[$key]['TIME_LOGGED']/3600) : null);
	$builder->td(isset($timeLogged[$key]['ISSUES']) ? round($timeLogged[$key]['TIME_LOGGED']/$timeLogged[$key]['ISSUES']/60) : null);

	if (!$report->isSensitive()) {
		$builder->td(isset($timeStatistics[$key]) ? round($timeStatistics[$key]['TIME_TO_SOLUTION']/3600/24,1) : null);
		$builder->td(isset($timeStatistics[$key]) ? round($timeStatistics[$key]['TIME_TO_ACTIVE']/3600/24,1) : null);
		$builder->td(isset($timeStatistics[$key]) ? round($timeStatistics[$key]['ACCURACY']/3600/24,1) : null);
		$builder->td(isset($feedback[$key]['RATING']) ? round($feedback[$key]['RATING']/$feedback[$key]['RATING_CNT']*100) : null);
		$builder->td(isset($feedback[$key]['RATING_CNT']) ? $feedback[$key]['RATING_CNT'] : null);
	}	
	
	$builder->tr();
}

$builder->th(tpl_text('Totals'));
$builder->sum(array(array(
	$builder->getCurrentCol(), 1,
	$builder->getCurrentCol(), count($groups))));
$builder->sum(array(array(
	$builder->getCurrentCol(), 1,
	$builder->getCurrentCol(), count($groups))));
$builder->sum(array(array(
	$builder->getCurrentCol(), 1,
	$builder->getCurrentCol(), count($groups))));
$builder->sum(array(array(
	$builder->getCurrentCol(), 1,
	$builder->getCurrentCol(), count($groups))));
$builder->sum(array(array(
	$builder->getCurrentCol(), 1,
	$builder->getCurrentCol(), count($groups))));
$builder->sum(array(array(
	$builder->getCurrentCol(), 1,
	$builder->getCurrentCol(), count($groups))));
$builder->sum(array(array(
	$builder->getCurrentCol(), 1,
	$builder->getCurrentCol(), count($groups))));
$builder->avg(array(array(
	$builder->getCurrentCol(), 1,
	$builder->getCurrentCol(), count($groups))), null, null, 0);

if (!$report->isSensitive()) {
	$builder->avg(array(array(
		$builder->getCurrentCol(), 1,
		$builder->getCurrentCol(), count($groups))), null, null, 1);
	$builder->avg(array(array(
		$builder->getCurrentCol(), 1,
		$builder->getCurrentCol(), count($groups))), null, null, 1);
	$builder->avg(array(array(
		$builder->getCurrentCol(), 1,
		$builder->getCurrentCol(), count($groups))), null, null, 1);
	$builder->avg(array(array(
		$builder->getCurrentCol(), 1,
		$builder->getCurrentCol(), count($groups))), null, null, 0);
	$builder->sum(array(array(
		$builder->getCurrentCol(), 1,
		$builder->getCurrentCol(), count($groups))));
}
