<? if (!isset($skip) || $node->nodeId != $skip->nodeId) { ?>
<? $keywords = $node->getCategories(); ?>
<? $this->render($node,'list_view_option.tpl',array_merge((array)$_data,array('disabled' => $candisable && count($keywords)))) ?>
<? $this->iterate(SyndLib::sort(iterator_to_array($keywords->getIterator())),'option_expand_keywords.tpl',
	array_merge((array)$_data,array('pad'=>$pad+1))) ?>
<? } ?>