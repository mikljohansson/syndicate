<div class="Article">
	<h1><?= $node->toString() ?></h1>
	<? if (isset($_REQUEST['agent'])) { ?>
		<? if ('ok' == $_REQUEST['agent']) { ?>
		<div class="Success">
			<?= tpl_text('The agent responded and is running, updated information should be available within minutes.') ?>
		</div>
		<? } else if ('none' == $_REQUEST['agent']) { ?>
		<div class="Notice">
			<?= tpl_text('No agents were found on any of the registered IP-addresses.') ?>
		</div>
		<? } else { ?>
		<div class="Warning">
			<?= tpl_text("Unknown response '{$_REQUEST['agent']}' from agent.") ?>
		</div>
		<? } ?>
	<? } else if ($node->isListening()) { ?>
	<ul class="Actions">
		<li><a href="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'runAgent') ?>">
			<?= tpl_text('Attempt to launch the inventory agent') ?></a></li>
	</ul>
	<? } ?>

	<? 
	$pane = clone $this;

	$pane->append('tabs', array(
		'uri' => tpl_link($node->getHandler(),'view',$node->nodeId),
		'text' => tpl_text('Info'),
		'selected' => null == $request[0],
		'template' => array($node,'pane_view.tpl')));

	if (null != $node->data['INFO_RAM']) {
		$pane->append('tabs', array(
			'uri' => tpl_link($node->getHandler(),'view',$node->nodeId,'hardware'),
			'text' => tpl_text('Hardware'),
			'selected' => 'hardware' == $request[0],
			'template' => array($node,'pane_view_hardware.tpl')));
	}

	$os = SyndLib::sort($node->getOperatingSystems());
	foreach (array_keys($os) as $key) {
		$pane->append('tabs', array(
			'uri' => tpl_link($node->getHandler(),'view',$node->nodeId,'os',$os[$key]->nodeId),
			'text' => tpl_chop($os[$key]->toString(),20),
			'selected' => 'os' == $request[0] && $os[$key]->nodeId == $request[0],
			'template' => array($os[$key],'full_view.tpl')));
	}

	$pane->display(tpl_design_path('gui/pane/tabbed.tpl'));
	?>
</div>