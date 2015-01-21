<? 
$emails = array();

if ($node->isCustomerSelected()) 
	$emails[] = $this->text('Customer');
if ($node->isAssignedSelected()) 
	$emails[] = $this->text('Assigned');
$emails[] = $node->getCc();

print implode(', ', $emails);
?>