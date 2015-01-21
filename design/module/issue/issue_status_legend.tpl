<? $codes = Module::getInstance('issue')->getDefinedStatusCodes(); ?>
<div class="Issues">
	<div class="Legend">
		<span class="Pending"><span class="Status"><img src="<?= tpl_design_uri('image/pixel.gif') ?>" alt=" " /> <?= tpl_text($codes[synd_node_issue::PENDING][1]) ?></span></span>
		<span class="Recent"><span class="Status"><img src="<?= tpl_design_uri('image/pixel.gif') ?>" alt=" " /> <?= tpl_text($codes[synd_node_issue::RECENT][1]) ?></span></span>
		<span class="Active"><span class="Status"><img src="<?= tpl_design_uri('image/pixel.gif') ?>" alt=" " /> <?= tpl_text($codes[synd_node_issue::ACTIVE][1]) ?></span></span>
	</div>
</div>