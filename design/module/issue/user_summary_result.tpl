<?
	print tpl_translate('<a href="%s">%d open issues</a>', tpl_link('issue','report',array('customer'=>$user->getEmail(),'subissues'=>1,'output'=>array('html'=>1))), count($open));
	if (closed)
		print ', '.tpl_translate('<a href="%s">%d closed</a>', tpl_link('issue','report',array('customer'=>$user->getEmail(),'subissues'=>1,'status'=>'closed','output'=>array('html'=>1))), count($closed));
	if ($assigned)
		print ', '.tpl_translate('<a href="%s">%d assigned</a>', tpl_link('issue','report',array('assigned'=>$user->nodeId,'subissues'=>1,'output'=>array('html'=>1))), count($assigned));
?>, <a href="<?= tpl_link('issue','newIssue',array('customer'=>$user->nodeId)) ?>" accesskey="n" title="<?= tpl_text('Accesskey: %s','N') ?>"><?= tpl_text('create new issue') ?></a>
