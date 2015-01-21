<div class="Issue">
	<table>
		<tr>
			<td>
				<table class="Vertical">
					<tr class="<?= tpl_cycle(array('odd','even')) ?>">
						<th><?= $this->text('To') ?></th>
						<td><input type="text" name="to" id="to" value="<?= tpl_attribute($to) ?>" size="70" /></td>
						<th rowspan="5"><?= $this->text('Status') ?></th>
						<td rowspan="5">
							<? $this->render($node,'part_edit_status.tpl',array('data'=>$node->getCompositeData())) ?>
							<? if ($redirect) { ?>
							<h4><?= $this->text('Options') ?></h4>
							<?= tpl_form_checkbox('redirect',false,$redirect,null,array('onchange'=>"toggleRedirection(this);")) ?> 
								<label for="redirect" title="<?= $this->text('Redirect message verbatim') ?>"><?= $this->text('Redirect') ?></label>
							<? } ?>
						</td>
					</tr>
					<tr class="<?= tpl_cycle() ?>">
						<th><?= $this->text('Cc') ?></th>
						<td><input type="text" name="cc" id="cc" size="70" /></td>
					</tr>
					<tr class="<?= tpl_cycle() ?>">
						<th><?= $this->text('Bcc') ?></th>
						<td><input type="text" name="bcc" id="bcc" size="70" /></td>
					</tr>
					<tr class="<?= tpl_cycle() ?>">
						<th><?= $this->text('Subject') ?></th>
						<td><input type="text" name="subject" id="subject" value="<?= tpl_attribute($subject) ?>" size="70" /></td>
					</tr>
					<tr class="<?= tpl_cycle() ?>">
						<th><?= $this->text('Time spent') ?></th>
						<td><input type="text" name="duration" id="duration" value="10" size="70" /></td>
					</tr>
					<tr class="<?= tpl_cycle() ?>">
						<th><?= $this->text('Message') ?></th>
						<td colspan="3">
							<?= tpl_form_textarea('message',synd_node_issue::wordwrap($body),array('id'=>'message','cols'=>105),30,10) ?>
						</td>
					</tr>
					<? if (!empty($files)) { ?>
					<tr class="<?= tpl_cycle() ?>">
						<th><?= $this->text('Attachments') ?></th>
						<td colspan="3">
							<table>
								<thead>
									<td colspan="3">&nbsp;</td>
									<td style="width:1%;" class="Info" title="<?= $this->text('Attach files to outgoing e-mail') ?>"><?= $this->text('Attach') ?></td>
								</thead>
								<? foreach (array_keys($files) as $key) { ?>
								<tr>
									<td><a href="<?= $files[$key]->uri() ?>"><?= $files[$key]->toString() ?></a></td>
									<td class="Numeric" style="width:6em;"><?= $this->text('%dKb',ceil($files[$key]->getSize()/1024)) ?></td>
									<td class="Numeric" style="width:10em;"><?= ucwords(tpl_strftime('%d %b %Y %R',$files[$key]->getCreated())) ?></td>
									<td><?= tpl_form_checkbox('attachment[]',false,$files[$key]->id()) ?></td>
								</tr>
								<? } ?>
							</table>
						</td>
					</tr>
					<? } ?>
				</table>

				<p>
					<span title="<?= $this->text('Accesskey: %s','S') ?>">
						<input accesskey="s" type="submit" name="post" value="<?= $this->text('Send') ?>" />
					</span>
					<span title="<?= $this->text('Accesskey: %s','A') ?>">
						<input accesskey="a" type="button" value="<?= $this->text('Abort') ?>" onclick="window.location='<?= tpl_uri_return() ?>';" />
					</span>
				</p>
			</td>
		</tr>
	</table>
</div>
<script type="text/javascript">
<!--
	function toggleRedirection(checkbox) {
		if (document.getElementById) {
			document.getElementById('cc').disabled = checkbox.checked;
			document.getElementById('bcc').disabled = checkbox.checked;
			document.getElementById('subject').disabled = checkbox.checked;
			document.getElementById('duration').disabled = checkbox.checked;
			document.getElementById('message').disabled = checkbox.checked;
		}
	}
//-->
</script>
