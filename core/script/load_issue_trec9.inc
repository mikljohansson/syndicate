<?php
require_once 'core/lib/Mime.class.inc';
require_once 'core/lib/HttpConnection.class.inc';
require_once 'core/model/storage/DatabaseStorage.class.inc';

/**
 * @access	private
 */
function parseArguments($args) {
	$result = array(0 => $args[0]);
	$j = 1;
	
	for ($i=1; $i<count($args); $i++) {
		if ('-' == substr($args[$i],0,1) && isset($args[$i+1])) {
			$result[$args[$i]] = $args[$i+1];
			$i++;
		}
		else
			$result[$j++] = $args[$i];
	}
	
	return $result;
}

$args = parseArguments($argv);
if (!isset($args['--trec9']) || !is_file($args['--trec9']))
	return trigger_error('Please specify the location of the TREC9 ohsumed database using --trec9', E_USER_ERROR);

set_include_path(get_include_path().':'.realpath(dirname(__FILE__).'/../../'));
include_once 'synd.inc';

assert_options(ASSERT_BAIL, true);
assert_options(ASSERT_ACTIVE, true);
assert_options(ASSERT_WARNING, true);
error_reporting(E_ALL);

$storage = new DatabaseStorage($synd_maindb);
$project = $storage->factory('project');
$project->save();
$storage->flush();

$this->_fp = fopen($args['--trec9'], 'r');
fgets($this->_fp, 8192);

$count = 0;
$start = time();
$issue = $storage->factory('issue');
$issue->data['PARENT_NODE_ID'] = $project->nodeId;
$isIssue = true;
$id = $issue->nodeId;
$pid = $project->nodeId;

while (!feof($this->_fp)) {
	$line = fgets($this->_fp, 8192);
	
	if ('.I' == substr($line,0,2)) {
		$issue->save();
		if (!$issue->flush())
			trigger_error('Error on flush', E_USER_ERROR);
		$count++;

		if (0 == $count%100 && time()-$start)
			printf("Loaded %d issues in %d minutes (%.2fTPM)\n", $count, (time()-$start)/60, $count/((time()-$start)/60));
		
		unset($issue);

//		$issue = $storage->factory('project');
//		if (1 == rand(1,4))
//			$issue->data['PARENT_NODE_ID'] = 'null.issue';
//		else {
//			$sql = "
//				SELECT node_id FROM synd_project
//				ORDER BY RAND()";
//			$issue->data['PARENT_NODE_ID'] = $storage->_db->getOne($sql);
//		}

		if (1 == rand(1,3)) {
			if (1 == rand(1,10))
				$pid = $synd_maindb->getOne('SELECT node_id FROM synd_project ORDER BY RAND()');
			
			$issue = $storage->factory('issue');
			$issue->data['PARENT_NODE_ID'] = $pid;
			$id = $issue->nodeId;
			$isIssue = true;
		}
		else {
			$issue = $storage->factory('task');
			$issue->data['PARENT_NODE_ID'] = $id;
			$isIssue = false;
		}
	}
	else {
		$text = trim(fgets($this->_fp, 8192));
		switch (substr($line,0,2)) {
			case '.T':
				if ($isIssue)
					$issue->setTitle(substr($text,0,25));
				break;
			case '.W':
				$issue->setDescription(substr($text,0,255));
				break;
			case '.A':
				if ($isIssue)
					$issue->data['CLIENT_NODE_ID'] = 'user_case.'.substr($text,0,25);
				$issue->data['CREATE_NODE_ID'] = 'user_case.'.substr($text,0,25);
				$issue->data['UPDATE_NODE_ID'] = 'user_case.'.substr($text,0,25);
				break;
		}
	}
	
	unset($line, $text);
}

printf("Loaded %d issues in %d minutes (%d TPM)\n", $count, (time()-$start)/60, $count/(time()-$start)*60);
