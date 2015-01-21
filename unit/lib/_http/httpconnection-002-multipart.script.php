<? 
//print_r($_REQUEST);
//print_r($_FILES);
foreach ($_POST as $key => $value)
	print "$key $value\n";
foreach ($_FILES as $key => $value)
	print "$key {$value['name']} ".(@file_get_contents($value['tmp_name']))."\n";
