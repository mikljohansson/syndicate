	<? if (count($node->getFilterOptions())) { ?>
	<div class="Notice">
		<div style="margin-bottom:0.5em;">
			<?= tpl_text('%d replies matching current filters.', $node->getFilteredCount()) ?>
		</div>
		
		<? foreach ($node->getFilterOptions() as $id => $options) { ?>
			<? if (null != ($question = SyndNodeLib::getInstance($id))) { ?>
			<? $this->render($question,'filter.tpl',array('options' => $options)) ?>
			<? } ?>
		<? } ?>
		
		<div class="Info">
			<?= tpl_text('Filters limit the selection of replies that is used to generate statistics, i.e. filtering on Choice1 of Question1 limits the statistics to just those who selected that choice. To remove filters press the <em>Trash</em> icon.') ?>
		</div>
	</div>
	<? } ?>
