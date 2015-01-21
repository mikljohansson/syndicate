<option value="<?= $node->nodeId ?>"<?= $node->nodeId==$selected->nodeId?' selected="selected"':'' ?>>
	<?= strip_tags($this->fetchnode($node->getCustomer(),'head_view.tpl')) ?>
</option>
