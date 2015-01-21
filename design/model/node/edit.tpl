<form action="<?= $_SERVER['REQUEST_URI'] ?>" method="post" enctype="multipart/form-data">
	<? $this->render($node, $view, $_data) ?>
</form>