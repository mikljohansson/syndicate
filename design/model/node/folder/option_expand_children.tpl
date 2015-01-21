<? $this->render($node,'list_view_option.tpl',$_data) ?>
<? $this->iterate(SyndLib::sort(SyndLib::filter($node->getFolders(),'isPermitted','read'),'toString'),
	'option_expand_children.tpl',array_merge($_data,array('pad'=>$pad+1))) ?>