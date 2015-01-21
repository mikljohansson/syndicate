<? 
$options = array_merge(array(null => '&nbsp;'), array_combine(
	SyndLib::array_collect($node->getOptions(),'OPTION_NODE_ID'),
	SyndLib::array_collect($node->getOptions(),'INFO_OPTION')));

$select = '<select name="'.$node->getName($attempt).'">';
$select .= tpl_form_options($options, isset($answer) ? reset($answer) : null);
$select .= '</select>';

if (isset($answer))
	$select .= $this->fetchnode($node,'explanation.tpl',$_data);

?>
<span class="Inline">
	<?= preg_replace('/<(\d+|)>/', $select, $node->getQuestion()) ?>
</span>
