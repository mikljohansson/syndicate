<? global $synd_maindb; ?>
<h2><?= tpl_text('Serial number duplicates') ?></h2>

<?
$sql = "
	SELECT i.node_id FROM synd_inv_item i, (

		SELECT LOWER(i2.info_serial_maker) info_serial_maker 
		FROM synd_inv_item i2
		GROUP BY LOWER(i2.info_serial_maker)
		HAVING count(*) > 1) t
	WHERE LOWER(i.info_serial_maker) = t.info_serial_maker";
$items = SyndNodeLib::getInstances($synd_maindb->getCol($sql,0));
tpl_sort_list($items,'item');

?>
<div class="Result"><?= tpl_text('Maker serial number duplicates') ?></div>
<?= tpl_gui_table('item',$items,'view.tpl') ?>

<br /><br />

<?
$sql = "
	SELECT i.node_id FROM synd_inv_item i, (
		SELECT LOWER(i2.info_serial_internal) info_serial_internal
		FROM synd_inv_item i2
		GROUP BY LOWER(i2.info_serial_internal)
		HAVING count(*) > 1) t
	WHERE LOWER(i.info_serial_internal) = t.info_serial_internal";
$items = SyndNodeLib::getInstances($synd_maindb->getCol($sql,0));
tpl_sort_list($items,'item');

?>
<div class="Result"><?= tpl_text('Internal serial number duplicates') ?></div>
<?= tpl_gui_table('item',$items,'view.tpl') ?>