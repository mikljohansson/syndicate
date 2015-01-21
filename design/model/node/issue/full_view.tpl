<? $node->raiseEvent(new synd_event_view($node)); ?>
<? $this->displayonce('module/issue/context.tpl'); ?>
<div class="Issue">
	<input type="hidden" name="selection[]" value="<?= $node->id() ?>" />
	<? include tpl_gui_path('synd_node_issue','confirm_email_status.tpl') ?>
	
	<div class="Header">
		<h1><?= htmlspecialchars(tpl_chop($node->getTitle(),50)) ?></h1>
		<div class="Info"><?
			$issue = $node->getParentIssue(); 
			if (!$issue->isNull() && $issue->isPermitted('read')) {
				print $this->translate('Issue number #%s, subissue of <a href="%s">#%s</a>',
					$this->quote($node->objectId()),
					tpl_link($issue->getHandler(),$issue->objectId()),
					$this->quote(tpl_chop($issue->toString(),50)));
			}
			else {
				print $this->text('Issue number #%s', $node->objectId());
			}
		?></div>
	</div>
	
	<? if ($node->isUnmappedCustomer()) { ?>
		<div class="Success">
			<table>
				<tr>
					<td>
						<?= $this->translate("A user has been specified for a previously unknown address, should '%s' be automatically mapped to the user %s &lt;%s&gt; in the future?", 
							tpl_email($node->data['INFO_INITIAL_QUERY']), 
							$this->quote($node->getCustomer()->toString()), 
							tpl_email($node->getCustomer()->getEmail())) ?>
					</td>
					<td>
						<form action="<?= tpl_link_call('issue','invoke',$node->nodeId,'mapCustomer') ?>" method="post">
							<input type="submit" name="yes" value="<?= $this->text('Yes') ?>" />
							<input type="submit" name="no" value="<?= $this->text('No') ?>" />
						</form>
					</td>
				</tr>
			</table>
		</div>
	<? } ?>
	
	<table>
		<tr>
			<td class="Properties"><? $this->render($node,'part_view_header.tpl') ?></td>
		</tr>
		<tr>
			<td>
				<?
				$this->append('tabs', array(
					'uri' => tpl_link($node->getHandler(),$node->objectId()),
					'text' => $this->text('Info'),
					'template' => array($node,'pane_view.tpl'),
					'selected' => null == $request[0]));

				$this->append('tabs', array(
					'uri' => tpl_link($node->getHandler(),$node->objectId(),'details'),
					'text' => $this->text('Details'),
					'template' => array($node,'pane_view_details.tpl'),
					'selected' => 'details' == $request[0]));
					
				$this->append('tabs', array(
					'uri' => tpl_link($node->getHandler(),$node->objectId(),'history'),
					'text' => $this->text('History'),
					'template' => array($node,'pane_view_history.tpl'),
					'selected' => 'history' == $request[0]));
				
				$this->display('gui/pane/tabbed.tpl', $_data);
				?>
			</td>
		</tr>
		<? $this->render($node,'part_view_issues.tpl',$_data) ?>
		<? 

		$notes = $node->getPublishedNotes();
		$count = count($notes);
		
		if ($count) { ?>
		<tr>
			<td>
				<table>
					<? if ($count) { 
						$limit = 25;
						if (null == ($order = tpl_sort_order('task')))
							$order = array('TS_CREATE', 0);
						$offset = (int)$request['offset']; ?>
					<thead>
						<tr>
							<th>
								<?= $this->text('Comments') ?>
								<? $this->display(tpl_design_path('gui/pager.tpl'),array('offset'=>$offset,'limit'=>$limit,'count'=>$count)) ?>
							</th>
							<th style="text-align:right;"><a href="<?= tpl_sort_uri('task','TS_CREATE') 
								?>"><img src="<?= tpl_sort_switch('task','TS_CREATE',tpl_design_uri('image/tree/sort-asc.gif'),tpl_design_uri('image/tree/sort-dsc.gif'),false) 
								?>" title="<?= $this->text('Sort comments on date') ?>" alt="<?= tpl_sort_switch('task','TS_CREATE','&darr;','&uarr;',false) ?>" /></a></th>
						</tr>
					</thead>
					<tbody>
						<tr class="odd">
							<td colspan="2">
								<? $this->iterate($notes->getIterator($offset, $limit, $order), 'full_view.tpl') ?>
							</td>
						</tr>
					</tbody>
					<? } ?>
				</table>
			</td>
		</tr>
		<? } ?>
	</table>

	<? if ($node->isPermitted('write')) { ?>
	<table width="80%">
	<tr>
		<td width="50%">
			<? if (!$node->isClosed()) { ?>
			<form action="<?= tpl_link_call($node->getHandler(),'invoke',$node->nodeId,'cancel') ?>" method="post">
				<input type="checkbox" name="confirm" id="confirm_issue_cancel" value="1" /> 
					<label for="confirm_issue_cancel"><?= $this->text('Check to') ?></label>
				<input type="submit" value="<?= $this->text('Cancel issue') ?>" />
			</form>
			<? } ?>
		</td>
		<td>
			<form action="<?= tpl_link($node->getHandler(),'edit',$node->nodeId) ?>" method="get">
				<input type="hidden" name="stack[]" value="<?= tpl_uri_call() ?>" />
				<input accesskey="e" title="<?= $this->text('Accesskey: %s','E') ?>" type="submit" value="<?= $this->text('Edit &gt;&gt;&gt;') ?>" />
			</form>
		</td>
	</tr>
	</table>
	<? } ?>
</div>
