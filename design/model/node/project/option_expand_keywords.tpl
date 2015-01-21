<? 
$projects = iterator_to_array(new MethodFilterIterator($node->getProjects()->getIterator(), 'isPermitted', 'read')); 
$keywords = $node->getLocalCategories(); 
?>
<? $this->render($node,'list_view_option.tpl',$_data) ?>
<? $this->iterate(SyndLib::sort($projects),'option_expand_keywords.tpl',
	array_merge((array)$_data,array('pad'=>$pad+1))) ?>
<? $this->iterate(SyndLib::sort($keywords),'option_expand_keywords.tpl',
	array_merge((array)$_data,array('pad'=>$pad+1))) ?>
