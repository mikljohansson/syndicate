<? tpl_load_script(tpl_design_uri('js/ole.js')) ?>
<div style="margin-top:20px; margin-bottom:20px;">
	<table style="width:100%;">
	<tr>
		<td class="Item">
			<div class="Header">
				<h3><?= $node->toString() ?></h3>
			</div>
			<div class="Abstract">
				<? $body = $node->getBody(); ?>
				<?= $body->toString() ?>
			</div>
		</td>

		<? if ($node->isPermitted('write')) { ?>
		<td style="padding-top:10px;" class="nowrap" align="right" valign="top">
			<input type="checkbox" name="selection[]" value="<?= $node->id() ?>" tabindex="100" />
			<a href="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>" tabindex="101"><img src="<?= tpl_design_uri('image/icon/edit.gif') ?>" border="0" /></a>
		</td>
		<? } ?>
	</tr>
	</table>

	<? if ($node->isPermitted('write')) { ?>
	<div>
		<? $this->iterate($node->getChildren(),'item_design.tpl',$_data); ?>
		<table width="100%">
		<tr>
			<td class="top" style="width:250px; padding:10px;">
				<ul class="Actions">
					<li><a href="<?= tpl_link_call($node->getHandler(),'insert','open_ended_text',
						array('data'=>array('PARENT_NODE_ID' => $node->nodeId))) ?>">
						<?= tpl_text('Add Fill-in-the-Blank question') ?></a></li>
					<li><a href="<?= tpl_link_call($node->getHandler(),'insert','multiple_choice',
						array('data'=>array('PARENT_NODE_ID' => $node->nodeId))) ?>">
						<?= tpl_text('Add Multiple-Choice question') ?></a></li>
				</ul>
			</td>
			<td class="top" style="padding:10px;">
				<? $page = $node->getPage(); ?>
				<ul class="Actions">
					<li><a href="javascript:synd_ole_call('<?= tpl_view_call($node->getHandler(),'cut',$page->nodeId) ?>')" 
						title="<?= tpl_text('Cut the selected questions from this questionnaire') 
						?>"><?= tpl_text('Cut selected') ?></a></li>
					<li><a href="javascript:synd_ole_call('<?= tpl_view_call($node->getHandler(),'copy',$page->nodeId) ?>')" 
						title="<?= tpl_text('Copy the selected questions') 
						?>"><?= tpl_text('Copy selected') ?></a></li>
					<li><a href="javascript:synd_ole_call('<?= tpl_view_call($node->getHandler(),'delete') ?>')" 
						title="<?= tpl_text('Delete the selected questions') 
						?>"><?= tpl_text('Delete selected') ?></a></li>
				</ul>
			</td>
		</table>
	</div>
	<? } ?>
</div>
<hr />
