<? if (!empty($errors)) { ?>
<ul class="Warning">
	<? 
	$seen = array();
	foreach ($errors as $error) {
		if (!is_string($error) && isset($error['msg']))
			$error = $error['msg'];
		
		if (is_string($error) && !in_array($error, $seen)) {
			print '<li>'.tpl_quote($error).'</li>';
			$seen[] = $error;
		}
	}
?>
</ul>
<? } ?>
