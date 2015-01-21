<div class="Item">
	<div>
		<? $this->render($node->getCustomer(),'head_view.tpl') ?>
	</div>
	<table>
		<? $this->render($node,'trow_view.tpl') ?>
	</table>
</div>