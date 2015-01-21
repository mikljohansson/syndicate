# generated <?= date('Y-m-d H:i') ?> as '<?= tpl_request_host() ?><?= $_SERVER['REQUEST_URI'] ?>'

<? 
require_once 'core/lib/SyndHTML.class.inc';
$prevFolder = null;
$spaces = array();

// Render all groups and hosts
foreach (array_keys($interfaces) as $key) { 
	$nic = $interfaces[$key]->getNetworkInterface();
	$computer = $nic->getParent();
	$folder = $computer->getParent();
	$client = $computer->getCustomer();
	
	if (null === $prevFolder || $prevFolder->nodeId != $folder->nodeId) {
		if (null !== $prevFolder && !empty($definitions)) { ?>
}

<? 
		}
		
		$prevFolder = $folder;
		if (!$folder->isNull() && null != ($name = preg_replace('/(^[^a-z]+)|[^\w]+/i', '', SyndHTML::translateNationalChars($folder->toString())))) {
			$definitions = SyndLib::filter($folder->getOptionalDefinitions(), 'isInheritedFrom', 'RemboField');
			$values = $folder->getOptionalValues();
		}
		
		if (!empty($definitions)) { ?>
# group name '<?= $folder->toString() ?>'
Group <?= $name ?> {
<?
			foreach ($options = array_intersect(SyndLib::invoke($definitions,'toString'), array_keys($values)) as $key2 => $option) { ?>
	<?= $definitions[$key2]->toString() ?> <?= $definitions[$key2]->quote($values[$option]) ?>

<?
			}
		?>

<?
		}
	}
	
	if (!empty($definitions)) { ?>
	Host <?= $nic->getMacAddress() ?> # <?= $interfaces[$key]->getHostname() ?><?= $client->isNull() ? '' : ', '.strtr("'".$client->toString()." (".$client->getContact().")'","\r\n\0","   ") ?>

<?
	}

} 

if (null !== $prevFolder && !empty($definitions)) { ?>
}
<? }