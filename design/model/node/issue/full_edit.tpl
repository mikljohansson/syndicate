<? 
tpl_load_script(tpl_design_uri('js/form.js')); 
$this->displayonce('module/issue/context.tpl');

$files = $node->getFiles();
if (null !== ($task = $node->_storage->getInstance($node->_task)))
	$files = array_merge($files, $task->getFiles());

?>
<div class="Issue">
	<input type="hidden" name="selection[]" value="<?= $node->id() ?>" />
	<div class="Header">
		<? if ($node->isNew()) { ?>
		<h1><?= $this->translate('New issue for %s', $this->fetchnode($node->getParent(),'head_view.tpl')) ?></h1>
		<? } else { ?>
		<h1><?= synd_htmlspecialchars(tpl_chop($node->getTitle(),50)) ?></h1>
		<? } ?>
		<div class="Info"><?
			$issue = $node->getParentIssue(); 
			if (!$issue->isNull() && $issue->isPermitted('read')) {
				print $this->translate('Issue number <a href="%s">#%s</a>, subissue of <a href="%s">#%s</a>',
					tpl_link($node->getHandler(),$node->objectId()),
					$this->quote($node->objectId()),
					tpl_link($issue->getHandler(),$issue->objectId()),
					$this->quote(tpl_chop($issue->toString(),50)));
			}
			else {
				print $this->translate('Issue number <a href="%s">#%s</a>',
					tpl_link($node->getHandler(),$node->objectId()),
					$this->quote($node->objectId()));
			}
		?></div>
	</div>
	<? include tpl_design_path('gui/errors.tpl'); ?>
	
	<table>
		<tr>
			<td class="Properties">
				<? $this->render($node,'part_edit_header.tpl',$_data) ?>
			</td>
		</tr>
		<tr>
			<td>
				<?
				$this->append('tabs', array(
					'uri' => tpl_link_jump($node->getHandler(),'edit',$node->nodeId),
					'text' => $this->text('Info'),
					'template' => array($node,'pane_edit.tpl'),
					'selected' => null == $request[0]));
				
				$this->append('tabs', array(
					'uri' => tpl_link_jump($node->getHandler(),'edit',$node->nodeId,'details'),
					'text' => $this->text('Details'),
					'template' => array($node,'pane_edit_details.tpl'),
					'selected' => 'details' == $request[0]));

				$this->append('tabs', array(
					'uri' => tpl_link_jump($node->getHandler(),'edit',$node->nodeId,'history'),
					'text' => $this->text('History'),
					'template' => array($node,'pane_edit_history.tpl'),
					'selected' => 'history' == $request[0]));

				SyndLib::runHook('issue_edit_pane', $node, $this, $request);
				$this->display('gui/pane/tabbed_form.tpl', $_data);
				?>
			</td>
		</tr>
		<? $this->render($node,'part_view_issues.tpl',$_data) ?>
		<tr>
			<td>
				<table class="Vertical">
					<tr class="<?= $this->cycle() ?>">
						<th><?= $this->text('New comment') ?></th>
						<td colspan="3"><?= tpl_form_textarea('data[task][content]',$data['task']['content'],
							array('style' => 'width:96%;', 'tabindex' => $this->sequence()),20,6,10) ?></td>
					</tr>
					<tr class="<?= $this->cycle() ?>">
						<th><?= $this->text('Minutes') ?></th>
						<td>
							<input type="text" name="data[task][INFO_DURATION]" id="data[task][INFO_DURATION]" value="<?= 
								$data['task']['INFO_DURATION'] ?>" tabindex="<?= $this->sequence() ?>" title="<?= 
								$this->text('Arithmetic expressions such as \'60*3+45\' is supported') ?>" match="\d+" message="<?= 
								$this->text('Please fill in a duration') ?>" depend="data[task][content]" /> 
						</td>
						<th rowspan="3"><?= $this->text('Attachments') ?><? if(isset($errors['file'])) print '<span style="color:red;">*</span>'; ?></th>
						<td rowspan="3" class="Files">
							<input type="hidden" name="MAX_FILE_SIZE" value="20000000" />
							<input type="file" name="data[file]" size="30" />
							<input type="submit" value="<?= $this->text('Attach') ?>" />
							<? $this->render($node,'attachment/edit.tpl',$_data) ?>
						</td>
					</tr>
					<tr class="<?= $this->cycle() ?>">
						<th><?= $this->text('E-mail to') ?></th>
						<td>
							<? $mailNotifier = $node->getMailNotifier(); 
							$clientattribs = array('tabindex'=>$this->sequence());
							if (!empty($data['task']['FLAG_PROTECTED']))
								$clientattribs['disabled'] = 'disabled';
							?>
							<?= tpl_form_checkbox('data[mail][client]',$mailNotifier->isRegistered('onchange',$node->getCustomer()) || !empty($data['mail']['client']),1,null,$clientattribs) ?> 
								<label for="data[mail][client]"><?= $this->text('Customer') ?></label>
							<?= tpl_form_checkbox('data[mail][assigned]',($mailNotifier->isRegistered('onchange',$node->getAssigned()) || !empty($data['mail']['assigned'])) && $data['CLIENT_NODE_ID'] != $data['ASSIGNED_NODE_ID'],1,null,array('tabindex'=>$this->sequence())) ?> 
								<label for="data[mail][assigned]"><?= $this->text('Assigned') ?></label>
							&nbsp;
							<b>Cc:</b> <input tabindex="<?= $this->sequence() ?>" type="text" name="data[mail][address]" value="<?= tpl_value($data['mail']['address']) ?>" />
						</td>
					</tr>
					<tr class="<?= $this->cycle() ?>">
						<th>
							<span title="<?= $this->text('Accesskey: %s','S') ?>">
								<input tabindex="<?= $this->sequence() ?>" accesskey="s" type="submit" name="post" value="<?= $this->text('Save') ?>" />
							</span>
							<span title="<?= $this->text('Accesskey: %s','A') ?>">
								<input tabindex="<?= $this->sequence() ?>" accesskey="a" type="button" value="<?= $this->text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
							</span>
						</th>
						<td>
							<?= tpl_form_checkbox('data[task][FLAG_PROTECTED]',$data['task']['FLAG_PROTECTED'],1,null,array('onclick'=>'toggleCustomer(this)')) ?>
							<label for="data[task][FLAG_PROTECTED]"><?= $this->text('Internal comment') ?></label>
						</td>
					</tr>
				</table>
			</td>
		</tr>
		<? 

		$notes = $node->getPublishedNotes();
		$count = count($notes);
		
		if ($count) { 
			$limit = 25;
			if (null == ($order = tpl_sort_order('task')))
				$order = array('TS_CREATE', 0);
			$offset = (int)$request['offset']; ?>
		<tr>
			<td>
				<? if (!empty($request['comments']) && $node->isPermitted('admin')) { ?>
				<table class="Vertical">
					<thead>
						<tr class="Header">
							<th>
								<?= $this->text('Comments') ?>
								<? $this->display(tpl_design_path('gui/pager.tpl'),array('offset'=>$offset,'limit'=>$limit,'count'=>$count)) ?>
							</th>
							<td style="text-align:right;"><a href="<?= tpl_sort_uri('task','TS_CREATE') 
								?>"><img src="<?= tpl_sort_switch('task','TS_CREATE',tpl_design_uri('image/tree/sort-asc.gif'),tpl_design_uri('image/tree/sort-dsc.gif'),false) 
								?>" title="<?= $this->text('Sort comments on date') ?>" alt="<?= tpl_sort_switch('task','TS_CREATE','&darr;','&uarr;',false) ?>" /></a></td>
						</tr>
					</thead>
					<tbody>
						<tr class="<?= $this->cycle() ?>">
							<th>&nbsp;</th>
							<td>
								<? $this->iterate($notes->getIterator($offset, $limit, $order), 'full_edit.tpl', $_data) ?>
							</td>
						</tr>
					</tbody>
				</table>
				<? } else { ?>
				<table>
					<thead>
						<tr class="Header">
							<th colspan="2">
								<?= $this->text('Comments') ?>
								<? if (empty($request['comments']) && $node->isPermitted('admin')) { ?>
								<span class="Info">(<a href="<?= tpl_link_jump('issue','edit',$node->nodeId,array('comments'=>1)) ?>" title="<?= $this->text('Accesskey: %s','E') ?>" accesskey="e"><?= $this->text('Edit all comments') ?></a>)</span>
								<? } ?>
								<? $this->display(tpl_design_path('gui/pager.tpl'),array('offset'=>$offset,'limit'=>$limit,'count'=>$count)) ?>
							</th>
							<td style="text-align:right;"><a href="<?= tpl_sort_uri('task','TS_CREATE') 
								?>"><img src="<?= tpl_sort_switch('task','TS_CREATE',tpl_design_uri('image/tree/sort-asc.gif'),tpl_design_uri('image/tree/sort-dsc.gif'),false) 
								?>" title="<?= $this->text('Sort comments on date') ?>" alt="<?= tpl_sort_switch('task','TS_CREATE','&darr;','&uarr;',false) ?>" /></a></td>
						</tr>
					</thead>
					<tbody>
						<tr class="<?= $this->cycle() ?>">
							<td colspan="3">
								<? $this->iterate($notes->getIterator($offset, $limit, $order), 'full_view.tpl', array('edit'=>true)) ?>
							</td>
						</tr>
					</tbody>
				</table>
				<? } ?>
			</td>
		</tr>
		<? } ?>
	</table>
</div>

<script type="text/javascript">
<!--
	function toggleCustomer(oNode) {
		if (document.getElementById) {
			var oEmail = document.getElementById('data[mail][client]');
			if (!oNode.checked) 
				oEmail.disabled = false;
			else {
				oEmail.disabled = true;
				oEmail.checked = false;
			}
		}
	}
	
	if (document.getElementById) {
		<? if ($node->isNew()) { ?>
		var node = document.getElementById('client');
		if (null == node /* || '' != node.value */)
			node = document.getElementById('data[INFO_HEAD]');
		if (null == node /*|| '' != node.value */)
			node = document.getElementById('data[task][content]');
		<? } else { ?>
		var node = document.getElementById('data[task][content]');
		<? } ?>
		
		if (null != node)
			node.focus();
	}
//-->
</script>
