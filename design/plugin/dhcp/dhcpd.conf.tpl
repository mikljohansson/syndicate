# generated <?= date('Y-m-d H:i') ?> as '<?= tpl_request_host() ?><?= $_SERVER['REQUEST_URI'] ?>'

<? 
$prevFolder = null;
$spaces = array();

// Render all groups and hosts
ob_start();
foreach (array_keys($interfaces) as $key) { 
	$nic = $interfaces[$key]->getNetworkInterface();
	$computer = $nic->getParent();
	$folder = $computer->getParent();
	$client = $computer->getCustomer();
	
	if (null === $prevFolder || $prevFolder->nodeId != $folder->nodeId) {
		if (null !== $prevFolder) { ?>
}

<? } ?>
# group name '<?= $folder->toString() ?>'
group {
<?
		if (!$folder->isNull()) {
			$definitions = $folder->getOptionalDefinitions();
			$values = $folder->getOptionalValues();
		}
		
		if (!$folder->isNull() && count($options = array_intersect(SyndLib::invoke($definitions,'toString'), array_keys($values)))) {
			foreach ($options as $key2 => $option) { 
				if (!$definitions[$key2]->isInheritedFrom('DhcpDatatype') || !$definitions[$key2]->getOptionCode())
					continue;
				
				$space = $definitions[$key2]->getOptionSpace();
				if (!isset($spaces[$space][$key2]))
					$spaces[$space][$key2] = $definitions[$key2];

			?>
	option <?= $definitions[$key2]->toString() ?> <?= $definitions[$key2]->quote($values[$option]) ?>;
<?
			}
			?>

<?
		}

		$prevFolder = $folder;
	}
	else { ?>

<?
	}
	
	?>
	host <? if (null != $interfaces[$key]->getHostname()) print $interfaces[$key]->getHostname().' '; ?>{
		hardware ethernet <?= $nic->getMacAddress() ?>;
		fixed-address <?= isset($_REQUEST['fixed-address']) && 'host' == $_REQUEST['fixed-address'] ? 
			gethostbyaddr($interfaces[$key]->getIpAddress()) : $interfaces[$key]->getIpAddress() ?>;
	}<?= $client->isNull() ? '' : strtr(" # ".$client->toString()." (".$client->getContact().")","\r\n\0","   ") ?>

<? 
} 

if (null !== $prevFolder) { ?>
}
<? } 

$fragment = ob_get_clean();

// Output datatypes
foreach ($spaces as $space => $options) {
	if (0 !== $space) {
		?>
option space <?= $space ?>;
<?
	}
	
	foreach (array_keys($options) as $key) {
		?>
option <?= $options[$key]->toString() ?> code <?= $options[$key]->getOptionCode() ?> = <?= $options[$key]->getPrimitiveType() ?>;
<?
	}
	
	?>

<?
}

print $fragment;