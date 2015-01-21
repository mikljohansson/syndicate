	<? if (count($children = $node->getChildren())) { ?>
		<? $attempt = $node->getTrainingAttempt(); /*$node->createDiagnosticAttempt(); */?>
	<form method="post">
		<? $this->iterate($children,'item.tpl',array('attempt'=>$attempt)) ?>
		<div style="margin:15px;">
			<input type="submit" value="<?= tpl_text('Check my answers') ?>" />
			<? if ($attempt->hasAnswers()) { ?>
			<input type="button" value="<?= tpl_text('Take the exercise again') 
				?>" onclick="window.location=window.location;" />
			<? } ?>
		</div>
	</form>
	<? } ?>
