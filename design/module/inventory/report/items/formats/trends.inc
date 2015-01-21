<?

$optimistic = $report->getAvailableItems(true);
$pessimistic = $report->getAvailableItems(false);

$groups = array_unique(array_merge(array_keys($optimistic), array_keys($pessimistic)));
$groups = array_combine($groups, $groups);
