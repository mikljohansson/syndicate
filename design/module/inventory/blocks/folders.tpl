<ul class="Menu">
	<? $this->iterate(SyndLib::sort($folders),'menu.tpl',array('expand'=>$expand)) ?>
</ul>