<?php
set_include_path(get_include_path().':'.dirname(dirname(dirname(__FILE__))).':'.dirname(dirname(dirname(dirname(__FILE__)))).'/synd');
require_once 'synd.inc';

// List of issues keyword names to clean up
$keywords = array('GDPR-gallring');

$sql = "
	SELECT i.node_id 
	FROM synd_issue i, synd_project p
	WHERE 
		i.project_node_id = p.node_id AND
		i.ts_update <= (".time()." - p.info_cleanup_cutoff_seconds)

	UNION ALL
	
	SELECT i.node_id 
	FROM synd_issue i, synd_issue_keyword ik, synd_keyword k 
	WHERE
		i.node_id = ik.issue_node_id AND
		k.node_id = ik.keyword_node_id AND
		k.info_head IN (".implode(',', $synd_maindb->quote($keywords)).")";

//$ids = $synd_maindb->getCol($sql, 0, 0, 1);
$ids = array('issue.1934540');
$issues = SyndNodeLib::getInstances($ids);
$module = Module::getInstance('issue');

foreach ($issues as $issue) {
	$module->cleanup($issue);
}

