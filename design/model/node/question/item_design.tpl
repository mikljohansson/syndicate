<table style="width:100%">
<tr>
	<? if (isset($questionNumber)) { ?>
	<td style="padding-right:5px; width:20px;"><b><?= $questionNumber++ ?>:</b></td>
	<? } ?>
	<td><? $this->render($node,'layout/'.$node->getLayout().'.tpl',$_data) ?></td>
	<? if ($node->isPermitted('write')) { ?>
	<td style="padding-top:10px;" class="nowrap" align="right" valign="top">
		<input type="checkbox" name="selection[]" value="<?= $node->id() ?>" tabindex="100" />
		<a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>" tabindex="101"><img src="<?= tpl_design_uri('image/icon/edit.gif') ?>" border="0" /></a>
	</td>
	<? } ?>
</tr>
</table>