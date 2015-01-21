<?= tpl_form_text('mplex[mplex;'.$workflow->id().';'.$node->id().'/setSelectedOption/][option]', $node->getDescription()) ?>
<select name="mplex[mplex;<?= $workflow->id() ?>;<?= $node->id() ?>/setRelativeToday/][relativetoday]">
	<?= tpl_form_options(array($this->text('Relative to date in issue'), $this->text('Relative to today')), (int)$node->getRelativeToday()) ?>
</select>
