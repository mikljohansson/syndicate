<? 
$options = $node->getOptions();
if ($this->path(get_class(SyndLib::reset($options)), 'option_expand_children.tpl', false)) { ?>
<select name="mplex[mplex;<?= $workflow->id() ?>;<?= $node->id() ?>/setSelectedOption/][option]">
	<?= $this->iterate($options,'option_expand_children.tpl',array('selected'=>$node->getSelectedOption())) ?>
</select>
<? } else { ?>
<select name="mplex[mplex;<?= $workflow->id() ?>;<?= $node->id() ?>/setSelectedOption/][option]">
	<?= tpl_form_options($options,$node->getSelectedOption()->nodeId) ?>
</select>
<? } ?>
