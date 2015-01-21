<?= tpl_form_htmlarea(
	$id, isset($content) ? $content : $node->toString(), $style,
	array('upload' => tpl_view('system','invoke',$node->id(),'upload'))) 
?>