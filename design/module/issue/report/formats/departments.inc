<?
require_once 'service_level_report.inc';
$organizations = array();

// Sort rows per organisation
foreach ($groups as $key => $group) {
	$level1 = $group->getOrganization();
	$level2 = $level1;
	$level3 = $level1;
	
	while (null !== ($parent = $level1->getParent()) && !$parent->isNull()) {
		$level3 = $level2;
		$level2 = $level1;
		$level1 = $parent;
	}
	
	$organizations[$level2->id()][$level3->id()][$key] = $group;
}

function _slr_department_sort($a, $b) {
	if (null != ($ao = SyndLib::getInstance($a)) && null != ($bo = SyndLib::getInstance($b)))
		return strcasecmp($ao->toString(), $bo->toString());
	return 0;
}

uksort($organizations, '_slr_department_sort');

$builder->th(tpl_text('Organization'));
$builder->th(tpl_text('Department'));
$builder->th(tpl_text('Issues'));
$builder->th(tpl_text('2nd line'));
$builder->th(tpl_text('Closed'));
$builder->th(tpl_text('Closed within 15 min'));
$builder->th(tpl_text('Open'));
$builder->th(tpl_text('Open (2nd line)'));
$builder->th(tpl_text('Time logged'));
$builder->th(tpl_text('Time per issue'));

$builder->tr();

// Output organizations and customers
foreach ($organizations as $id => $departments) {
	if (null == ($organization = SyndLib::getInstance($id)))
		continue;
		
	$builder->th(tpl_default($organization->toString(), tpl_text('Unknown')), 'H1');
	$builder->th('');
	
	// Build the list of department blocks
	$blocks = array();
	$offset = 1;
	
	foreach ($departments as $customers) {
		$blocks[] = array(
			$builder->getCurrentCol(), $builder->getCurrentRow()+$offset+1,
			$builder->getCurrentCol(), $builder->getCurrentRow()+$offset+count($customers));
		$offset += count($customers) + 2;
	}
	
	// Render the summary columns for the whole organization
	for ($i=0; $i<7; $i++) {
		if (0 == $i && !$organization->isNull())
			$builder->sum($blocks, 'H1', tpl_uri_merge(array('organisation'=>$organization->getIdentifier(),'report'=>'list')));
		else if (2 == $i && !$organization->isNull())
			$builder->sum($blocks, 'H1', tpl_uri_merge(array('organisation'=>$organization->getIdentifier(),'report'=>'list','status'=>'closed')));
		else if (4 == $i && !$organization->isNull())
			$builder->sum($blocks, 'H1', tpl_uri_merge(array('organisation'=>$organization->getIdentifier(),'report'=>'list','status'=>'open')));
		else
			$builder->sum($blocks, 'H1');
		
		foreach (array_keys($blocks) as $key) {
			$blocks[$key][0]++;
			$blocks[$key][2]++;
		}
	}
	
	$builder->th('');
	$builder->tr();

	foreach ($departments as $id => $customers) {
		if (null == ($department = SyndLib::getInstance($id)))
			continue;
		
		$builder->th(tpl_default($department->toString(), tpl_text('Unknown')), 'H2');
		$builder->th('', 'H2');

		$builder->sum(array(array(
			$builder->getCurrentCol(), $builder->getCurrentRow()+1,
			$builder->getCurrentCol(), $builder->getCurrentRow()+count($customers))), 'H2', 
			$department->isNull() ? null : tpl_uri_merge(array('organisation'=>$department->getIdentifier(),'report'=>'list')));
		$builder->sum(array(array(
			$builder->getCurrentCol(), $builder->getCurrentRow()+1,
			$builder->getCurrentCol(), $builder->getCurrentRow()+count($customers))), 'H2');
		$builder->sum(array(array(
			$builder->getCurrentCol(), $builder->getCurrentRow()+1,
			$builder->getCurrentCol(), $builder->getCurrentRow()+count($customers))), 'H2',
			$department->isNull() ? null : tpl_uri_merge(array('organisation'=>$department->getIdentifier(),'report'=>'list','status'=>'closed')));
		$builder->sum(array(array(
			$builder->getCurrentCol(), $builder->getCurrentRow()+1,
			$builder->getCurrentCol(), $builder->getCurrentRow()+count($customers))), 'H2');
		$builder->sum(array(array(
			$builder->getCurrentCol(), $builder->getCurrentRow()+1,
			$builder->getCurrentCol(), $builder->getCurrentRow()+count($customers))), 'H2',
			$department->isNull() ? null : tpl_uri_merge(array('organisation'=>$department->getIdentifier(),'report'=>'list','status'=>'open')));
		$builder->sum(array(array(
			$builder->getCurrentCol(), $builder->getCurrentRow()+1,
			$builder->getCurrentCol(), $builder->getCurrentRow()+count($customers))), 'H2');
		$builder->sum(array(array(
			$builder->getCurrentCol(), $builder->getCurrentRow()+1,
			$builder->getCurrentCol(), $builder->getCurrentRow()+count($customers))), 'H2');

		$builder->th('', 'H2');
		$builder->tr();

		foreach (array_keys($customers) as $key) {
			$email = null != $customers[$key]->getEmail() ? $customers[$key]->getEmail() : $customers[$key]->getLogin();

			$builder->td($customers[$key]->toString(), null, tpl_link('user','summary',$customers[$key]->nodeId));
			$level1 = $customers[$key]->getOrganization();
			$builder->td($level1->toString());

			$builder->td(isset($received[$key]) ? $received[$key] : null, null, 
				tpl_uri_merge(array('customer'=>$email,'report'=>'list')));
			$builder->td(isset($receivedSecondLine[$key]) ? $receivedSecondLine[$key] : null);

			$builder->td(isset($closed[$key]) ? $closed[$key] : null, null, 
				tpl_uri_merge(array('customer'=>$email,'report'=>'list','status'=>'closed')));
			$builder->td(isset($quick[$key]) ? $quick[$key] : null);

			$builder->td(isset($pending[$key]) ? $pending[$key] : null, null, 
				tpl_uri_merge(array('customer'=>$email,'report'=>'list','status'=>'open')));
			$builder->td(isset($pendingSecondLine[$key]) ? $pendingSecondLine[$key] : null);

			$builder->td(isset($timeLogged[$key]['TIME_LOGGED']) ? round($timeLogged[$key]['TIME_LOGGED']/3600,1) : null);
			$builder->td(isset($timeLogged[$key]['TIME_LOGGED']) ? round($timeLogged[$key]['TIME_LOGGED']/$timeLogged[$key]['ISSUES']/60) : null);

			$builder->tr();
		}

		$builder->tr();
	}
}
