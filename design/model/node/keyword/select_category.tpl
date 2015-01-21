<? if ($node->isMandatory()) { 
	$keywords = SyndLib::sort(iterator_to_array($node->getCategories()->getIterator())); ?>
	<select name="<?= $input ?>" style="width:auto;" message="<?= $this->text('Category %s must be selected', $node->toString()) ?>" match=".+">
		<? $this->render($node,'list_view_option.tpl',array_merge((array)$_data,array(
			'candisable' => true,
			'disabled' => count($keywords), 
			'selected' => $selected))) ?>
		<? $this->iterate($keywords, 'option_expand_keywords.tpl', array(
			'candisable' => true,
			'selected' => $selected, 
			'pad'=>$pad+1)) ?>
	</select>
<? } else { ?>
	<?= tpl_form_checkbox($input, isset($selected[$node->nodeId]), $node->nodeId, $node->nodeId, array('tabindex'=>$this->sequence())) ?> 
	<label for="<?= $node->nodeId ?>" title="<?= tpl_attribute($node->getDescription()) ?>"><?= $node->toString() ?></label>
<? } ?>
