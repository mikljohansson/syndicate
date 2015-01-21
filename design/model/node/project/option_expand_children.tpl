<? $projects = iterator_to_array(new MethodFilterIterator($node->getPublishedProjects()->getIterator(), 'isPermitted', 'read')); ?>
<? $this->render($node,'list_view_option.tpl',$_data) ?>
<? $this->iterate(SyndLib::sort($projects),'option_expand_children.tpl',
	array_merge((array)$_data,array('pad'=>$pad+1))) ?>