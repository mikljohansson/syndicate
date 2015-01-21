<form method="post">
	<div class="Notice">
		<h2><?= tpl_text('Please confirm') ?></h2>
		<p><?= tpl_text("Do you really want to remove the notification of <em>'%s &lt;%s&gt;'</em> from the project <em>'%s'</em>?",
			$listener->toString(),$listener->getEmail(),$node->toString()) ?></p>
		<p>
			<input type="submit" name="confirm" value="<?= tpl_text('Ok') ?>" />
			<input type="button" value="<?= tpl_text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
		</p>
	</div>
</form>