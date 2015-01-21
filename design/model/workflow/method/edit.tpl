<select name="mplex[mplex;<?= $workflow->id() ?>;<?= $node->id() ?>/setSelectedOption/][option]">
	<?= tpl_form_options(array_map(array($this,'text'),$node->getOptions()),$node->getSelectedOption()) ?>
</select>