<form action="<?= tpl_view('issue','merge',$issue->nodeId) ?>" enctype="multipart/form-data" method="post">
	<input type="hidden" name="collections[]" value="<?= $collection->id() ?>" />
	<? $this->render($issue,'full_edit.tpl',array('data'=>$data,'errors'=>$errors)) ?>
</form>
