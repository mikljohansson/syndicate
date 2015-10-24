	<div class="OptionalField">
		<h3><?= tpl_text('SVN repository location') ?></h3>
		<input type="text" name="data[attributes][svn_repository_location]" value="<?= tpl_attribute($repository) ?>" size="64" maxlength="1024" />
		<div class="Info"><?= tpl_text("Allows for integrating issues and SVN source code revisions. Provide the respository uri, for example <em>'http://svn.synd.info/synd/'</em>") ?></div>
	</div>
	<div class="OptionalField">
		<h3><?= tpl_text('ViewVC location') ?></h3>
		<input type="text" name="data[attributes][svn_viewvc_location]" value="<?= tpl_attribute($viewvc) ?>" size="64" maxlength="1024" />
		<div class="Info"><?= tpl_text("Allows for integrating ViewVC source code viewing. Provide the ViewVC base uri, for example <em>'http://svn.synd.info/cgi-bin/viewcvs.cgi/synd/'</em>") ?></div>
	</div>	