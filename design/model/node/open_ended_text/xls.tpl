<? 
header('Content-Type: application/xls');
foreach ($node->getAnswers() as $answer) 
	print "\"".preg_replace('/[\r\n]+/', ' ', str_replace('"','\"',$answer['INFO_ANSWER']))."\"\r\n";
?>