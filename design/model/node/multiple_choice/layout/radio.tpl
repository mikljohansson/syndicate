<div class="MultipleChoice">
	<div class="Abstract">
		<?= $node->getQuestion() ?>
		<? if (isset($answer)) { ?>
		<? $this->render($node,'explanation.tpl',$_data) ?>
		<? } ?>
	</div>
	<div class="indent">
		<? 
		$options = $node->getOptions();
		foreach ($options as $option) { ?>
			<? $this->render($node,'option.tpl', array_merge($_data, array(
				'type' => 'radio',
				'option' => $option, 
				'label' => $option['INFO_OPTION']))) ?><br />
		<? } ?>
	</div>
</div>