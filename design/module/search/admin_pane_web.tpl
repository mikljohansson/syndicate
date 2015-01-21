<? 
$search = Module::getInstance('search');
$flags = $search->getSpiderFlags(); 

?>
<form action="<?= tpl_link_call('search','setSpiderOptions') ?>" method="post">
	<h3><?= tpl_text('Index maintenance') ?></h3>
	<div class="Info">
		<?= tpl_text('Last spider crawl <em>%s</em>.', tpl_date('Y-m-d H:i', $search->getVariable('lastSpiderCrawl'), tpl_text('Never'))) ?>
		<? if ($search->getVariable('lastSpiderCrawl') && $search->getVariable('lastSpiderCrawlFinished')) { ?>
			<?= tpl_text('Task duration <em>%s</em>.', tpl_duration($search->getVariable('lastSpiderCrawlFinished') - $search->getVariable('lastSpiderCrawl'))) ?>
		<? } ?>
	</div>
	<br />
	<?= tpl_form_checkbox('flags[active]', $flags['active']) ?>
		<label for="flags[active]"><?= tpl_text('Start the Spider process each night to refresh index (via synd/cron.php)') ?></label><br />

	<br />
	<h3><?= tpl_text('URI filters') ?></h3>
	<p class="Info">
		<?= tpl_text("Add domains that should be spidered, for example <em>'example.com'</em>. The domain is compiled to a regexp filter used by the spider to determine whether a page should be indexed.") ?>
	</p>
	<table>
	<? foreach ($search->getSpiderURIFilters() as $key => $pattern) { ?>
		<tr>
			<td><code><?= $pattern ?></code></td>
			<td>&nbsp;&nbsp;<a href="<?= tpl_link_call('search','delSpiderURIFilter',array('key'=>$key)) 
				?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" /></a></td>
		</tr>
	<? } ?>
	</table>
	<br />
	
	<input type="text" name="filter[domain]" size="60" />
	<input type="submit" value="<?= tpl_text('Save') ?>" /><br />
	<?= tpl_form_checkbox('filter[subdomains]',true) ?>
		<label for="filter[subdomains]"><?= tpl_text('Include subdomains') ?></label><br />
	
	<br />
	<h3><?= tpl_text('Starting points') ?></h3>
	<p class="Info">
		<?= tpl_text("Add a list of initial URIs to load into the Spider. For example; when spidering the domain <em>'example.com'</em> one would add its default homepage <em>'http://www.example.com/'</em>.") ?>
	</p>
	<table>
	<? foreach ($search->getSpiderPreloadLocations() as $key => $uri) { ?>
		<tr>
			<td><code><?= $uri ?></code></td>
			<td>&nbsp;&nbsp;<a href="<?= tpl_link_call('search','delSpiderPreloadLocation',array('key'=>$key)) 
				?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" /></a></td>
		</tr>
	<? } ?>
	</table>
	<br />

	<input type="text" name="preload" size="60" />
	<input type="submit" value="<?= tpl_text('Save') ?>" /><br />

	<br />
	<h3><?= tpl_text('Content-Type filters') ?></h3>
	<p class="Info">
		<?= tpl_text("Add Content-Type's that should be indexed, for example <em>'application/pdf'</em>. The content-type is compiled to a regexp filter used by the spider to determine whether a page should be indexed.") ?>
	</p>
	<table>
	<? foreach ($search->getContentTypeFilters() as $key => $pattern) { ?>
		<tr>
			<td><code><?= $pattern ?></code></td>
			<td>&nbsp;&nbsp;<a href="<?= tpl_link_call('search','delSpiderContentTypeFilter',array('key'=>$key)) 
				?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" /></a></td>
		</tr>
	<? } ?>
	</table>

	<input type="text" name="filter[content]" size="60" />
	<input type="submit" value="<?= tpl_text('Save') ?>" /><br />
	<br />
	
	<?= tpl_form_checkbox('flags[content][pdf]',!empty($flags['content']['pdf'])) ?>
		<label for="flags[content][pdf]"><?= tpl_text('Adobe PDF documents (.pdf)') ?></label><br />
	<?= tpl_form_checkbox('flags[content][doc]',!empty($flags['content']['doc'])) ?>
		<label for="flags[content][doc]"><?= tpl_text('Microsoft Word documents (.doc)') ?></label><br />
	<?= tpl_form_checkbox('flags[content][xls]',!empty($flags['content']['xls'])) ?>
		<label for="flags[content][xls]"><?= tpl_text('Microsoft Excel spreadsheets (.xls)') ?></label><br />
	<?= tpl_form_checkbox('flags[content][ppt]',!empty($flags['content']['ppt'])) ?>
		<label for="flags[content][ppt]"><?= tpl_text('Microsoft PowerPoint presentations (.ppt)') ?></label><br />
	<?= tpl_form_checkbox('flags[content][sxw]',!empty($flags['content']['sxw'])) ?>
		<label for="flags[content][sxw]"><?= tpl_text('OpenOffice.org documents (.sxw)') ?></label><br />
	
	<br />
	<input type="submit" value="<?= tpl_text('Save') ?>" />
</form>