<? require_once 'core/lib/SyndDate.class.inc'; ?>
<? $path = "node/edit/$node->nodeId/"; ?>
<div class="Task">
	<input type="hidden" name="mplex[<?= $path ?>][post]" value="1" />
	<table>
		<tr>
			<td rowspan="2">
				<? $path2 = tpl_view($node->id(),'setDescription'); $content = $node->getContent(); ?>
				<?= tpl_form_textarea("mplex[$path2][description]",isset($_REQUEST['mplex'][$path2][0]) ?
					$_REQUEST['mplex'][$path2][0] : $content->toString(), array('style'=>'width:96%')) ?>
			</td>
			<td style="height:2em;"><?= tpl_text('Minutes') ?></td>
			<td>
				<input type="text" name="mplex[<?= $path ?>][data][INFO_DURATION]" value="<?= 
					SyndDate::durationExpr(tpl_value($_REQUEST['mplex'][$path]['data']['INFO_DURATION'], 
					$node->data['INFO_DURATION'])) ?>" size="10" />
			</td>
			<td rowspan="2">
				<a href="<?= tpl_link_call('issue','delete',$node->nodeId) ?>"><img src="<?= 
					tpl_design_uri('image/icon/trash.gif') ?>" width="16" height="16" alt="<?= tpl_text('Delete') ?>" /></a>
			</td>
		</tr>
		<tr>
			<td><?= tpl_text('Options') ?></td>
			<td>
				<?= tpl_form_checkbox("mplex[$path][data][FLAG_PROTECTED]",
					tpl_value($_REQUEST['mplex'][$path]['data']['FLAG_PROTECTED'],$node->data['FLAG_PROTECTED']),1,null,
					array('tabindex'=>$tabindex++ + 1)) ?>
					<label for="mplex[<?= $path ?>][data][FLAG_PROTECTED]"><?= tpl_text('Internal comment') ?></label><br />
			</td>
		</tr>
	</table>
	<? $this->render($node->getParent(),'attachment/edit.tpl',array_merge((array)$_data, array('node'=>$node))) ?>
</div>
