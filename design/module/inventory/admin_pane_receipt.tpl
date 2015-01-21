<? $inventory = Module::getInstance('inventory'); ?>

<table>
	<tr>
		<td style="width:85%; padding-right:1em;">
			<h3><?= tpl_text('Lease templates') ?></h3>
			<p class="Help"><?= tpl_text('These templates are used when printing receipts from leases') ?></p>
			<div style="margin:1em;">
				<? if (count($templates = $inventory->getTemplates('synd_node_lease'))) { ?>
				<? $this->iterate($templates,'item.tpl') ?>
				<? } else { ?><em><?= tpl_text('No templates specified') ?></em><? } ?>
			</div>

			<h3><?= tpl_text('Repair templates') ?></h3>
			<p class="Help"><?= tpl_text('These templates are used when printing receipts from simple repair issues') ?></p>
			<div style="margin:1em;">
				<? if (count($templates = $inventory->getTemplates('synd_node_repair'))) { ?>
				<? $this->iterate($templates,'item.tpl') ?>
				<? } else { ?><em><?= tpl_text('No templates specified') ?></em><? } ?>
			</div>

			<h3><?= tpl_text('Replacement templates') ?></h3>
			<p class="Help"><?= tpl_text('These templates are used when printing receipts from replacement issues') ?></p>
			<div style="margin:1em;">
				<? if (count($templates = $inventory->getTemplates('synd_node_replace'))) { ?>
				<? $this->iterate($templates,'item.tpl') ?>
				<? } else { ?><em><?= tpl_text('No templates specified') ?></em><? } ?>
			</div>

			<h3><?= tpl_text('Lease termination templates') ?></h3>
			<p class="Help"><?= tpl_text('These templates are used when printing receipts from lease termination issues') ?></p>
			<div style="margin:1em;">
				<? if (count($templates = $inventory->getTemplates('synd_node_terminate'))) { ?>
				<? $this->iterate($templates,'item.tpl') ?>
				<? } else { ?><em><?= tpl_text('No templates specified') ?></em><? } ?>
			</div>

			<h3><?= tpl_text('Invoice templates') ?></h3>
			<p class="Help"><?= tpl_text('These templates are used when printing invoices') ?></p>
			<div style="margin:1em;">
				<? if (count($templates = $inventory->getTemplates('synd_node_invoice'))) { ?>
				<? $this->iterate($templates,'item.tpl') ?>
				<? } else { ?><em><?= tpl_text('No templates specified') ?></em><? } ?>
			</div>

			<h3><?= tpl_text('Move items templates') ?></h3>
			<p class="Help"><?= tpl_text('These templates are used when moving items between folders using the batch dialogue') ?></p>
			<div style="margin:1em;">
				<? if (count($templates = $inventory->getTemplates('move'))) { ?>
				<? $this->iterate($templates,'item.tpl') ?>
				<? } else { ?><em><?= tpl_text('No templates specified') ?></em><? } ?>
			</div>

			<form action="<?= tpl_view_call('inventory','addTemplate') ?>" enctype="multipart/form-data" method="post">
				<input type="hidden" name="MAX_FILE_SIZE" value="8000000" />
				<select name="template">
					<option value="synd_node_lease"><?= tpl_text('Lease template') ?></option>
					<option value="synd_node_repair"><?= tpl_text('Repair template') ?></option>
					<option value="synd_node_replace"><?= tpl_text('Replacement template') ?></option>
					<option value="synd_node_terminate"><?= tpl_text('Lease termination template') ?></option>
					<option value="synd_node_invoice"><?= tpl_text('Invoice template') ?></option>
					<option value="move"><?= tpl_text('Move items template') ?></option>
				</select>
				<input type="file" name="file" size="60" /> <input type="submit" value="<?= tpl_text('Add') ?>" />
			</form>
		</td>
		<td>
			<p class="Help"><?= tpl_text('The receipts are assumed to be in PDF format with named form-fields which the values will be substituted into using FDF generation and subsequenct merging.') ?></p>
			<p class="Help"><?= tpl_text('The merger is done using pdftk which the server must have installed, see <a href="http://www.accesspdf.com/pdftk/">http://www.accesspdf.com/pdftk/</a> and <em>synd/core/lib/SyndPrint.class.inc</em> for more information.') ?></p>
		</td>
	</tr>
</table>


<h3><?= tpl_text('Printer options') ?></h3>
<? if (count($printers = $inventory->getPrinters())) { ?>
<table>
<? foreach ($printers as $printer) { ?>
	<tr>
		<td style="padding-right:1em;"><?= $printer ?></td>
		<td><a href="<?= tpl_link_call('inventory','delPrinter',array('printer'=>$printer)) 
			?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" /></a></td>
	</tr>
<? } ?>
</table>
<? } ?>

<form action="<?= tpl_view_call('inventory','addPrinter') ?>" method="post">
	<input type="text" name="printer" /> <input type="submit" value="<?= tpl_text('Add') ?>" />
	<p class="Help"><?= tpl_text("Adding several printers allow the user to select the most convenient one, when printing receipts from the various inventory routines. Printers should have the form of laserjet1@server and printing will be done like '/usr/bin/lpr -Plaserjet1@server' (alt. 192.168.1.1%%9100 to print directly to printer on jetdirect port)") ?></p>
</form>

<h3><?= tpl_text('Search preprocessor') ?></h3>
<table>
	<tr>
		<th><?= tpl_text('Pattern') ?></th>
		<th><?= tpl_text('Replacement') ?></th>
		<th>&nbsp;</th>
	</tr>
	<? foreach ($inventory->getSearchDirectives() as $key => $directive) { ?>
	<tr class="<?= tpl_cycle(array('odd','even')) ?>">
		<td style="padding:5px;"><?= $directive['pattern'] ?></td>
		<td style="padding:5px;"><?= $directive['replacement'] ?></td>
		<td style="padding:5px;"><a href="<?= tpl_link_call('inventory','delSearchPattern',array('key'=>$key)) ?>"><img border="0" src="<?= tpl_design_uri('image/icon/trash.gif') ?>" /></a></td>
	</tr>
	<? } ?>
</table>

<form action="<?= tpl_view_call('inventory','addSearchPattern') ?>" method="post">
	<input type="text" name="pattern" />
	<input type="text" name="replacement" />
	<input type="submit" value="<?= tpl_text('Add') ?>" />
	<p class="Help">
		<?= tpl_text('The preprocessor consists of regular expressions that search queries is fed through via preg_replace(); before being executed. This can be useful for parsing and splitting the output from barcode scanners.') ?>
		<?= tpl_translate('See %s for more info', '<a href="http://www.php.net/preg_replace">http://www.php.net/preg_replace</a>') ?>
	</p>
</form>
